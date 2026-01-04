#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/shape.h>

namespace mjs {

Object::Object(Runtime* runtime, ClassId class_id) {
	// 挂入gc链表
	runtime->gc_manager().AddObject(this);

	tag_.class_id_ = static_cast<uint16_t>(class_id);
	tag_.is_extensible_ = 1;  // 默认可扩展
	shape_ = &runtime->shape_manager().empty_shape();

	// runtime的对象，只允许在初始化阶段变更，因此引用和解引用不考虑线程安全问题
	shape_->Reference();
}

Object::Object(Context* context, ClassId class_id) {
	// 挂入gc链表
	context->gc_manager().AddObject(this);

	tag_.class_id_ = static_cast<uint16_t>(class_id);
	tag_.is_extensible_ = 1;  // 默认可扩展
	shape_ = &context->shape_manager().empty_shape();
	shape_->Reference();
}

Object::~Object() {
	assert(gc_mark() || tag_.ref_count_ == 0);
	//if (property_map_) {
	//	delete property_map_;
	//}
	shape_->Dereference();
}


void Object::SetProperty(Runtime* runtime, ConstIndex key, Value&& value) {
	SetProperty(static_cast<Context*>(nullptr), key, std::move(value));
}

bool Object::GetProperty(Runtime* runtime, ConstIndex key, Value* value) {
	// 如果配置了exotic，需要先查找(也就是class_def中的GetProperty等方法)

	// 1. 查找自身属性
	auto index = shape_->Find(key);
	if (index != kPropertySlotIndexInvalid) {
		*value = GetPropertyValue(index);
		return true;
	}

	// 2. 原型链查找
	auto& prototype = GetPrototype(runtime);
	if (prototype.IsObject()) {
		auto success = prototype.object().GetProperty(runtime, key, value);
		if (success) return true;
	}

	return false;
}

void Object::SetProperty(Context* context, ConstIndex key, Value&& value) {
	// 检查属性是否存在
	auto index = shape_->Find(key);

	if (index == kPropertySlotIndexInvalid) {
		// 新属性：检查对象是否可扩展
		if (!tag_.is_extensible_) {
			// 对象不可扩展（包括 frozen, sealed, preventExtensions），不能添加新属性
			return;
		}
	}
	else {
		// 属性已存在：检查对象状态和属性级别的标志
		uint32_t prop_flags = GetPropertyFlags(index);

		// 情况 1：属性是 setter
		if (prop_flags & ShapeProperty::kIsSetter) {
			// 自动调用 setter，this 绑定到当前对象，value 作为参数
			Value argv[] = { std::move(value) };
			context->CallFunction(&GetPropertyValue(index), Value(this), std::begin(argv), std::end(argv));
			return;
		}

		// 情况 2：属性是 getter（只读访问器）
		if (prop_flags & ShapeProperty::kIsGetter) {
			// 尝试写入只有getter的属性应该失败
			return;
		}

		// 情况 3：检查属性级别的 writable 标志
		if (!(prop_flags & ShapeProperty::kWritable)) {
			// 只读属性，静默忽略
			return;
		}

		// 可写的普通属性，直接更新值
		SetPropertyValue(index, std::move(value));
		return;
	}

	// 添加新属性，使用默认标志（包含 enumerable, configurable, writable）
	uint32_t default_flags = ShapeProperty::kDefault;
	index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(key));
	AddPropertySlot(index, std::move(value), default_flags);
}

void Object::SetPropertyWithFlags(Context* context, ConstIndex key, Value&& value, uint32_t flags) {
	// 如果对象不可扩展且属性不存在，静默忽略
	auto index = shape_->Find(key);
	if (index == kPropertySlotIndexInvalid) {
		// 对象不可扩展，不能添加新属性
		if (!tag_.is_extensible_) {
			return;
		}
	}

	// 添加或更新属性
	index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(key));
	AddPropertySlot(index, std::move(value), flags);
}

void Object::DefineAccessorProperty(Context* context, ConstIndex key,
                                     FunctionObject* getter,
                                     FunctionObject* setter,
                                     uint32_t flags) {
	// 如果对象不可扩展且属性不存在，静默忽略
	auto index = shape_->Find(key);
	if (index == kPropertySlotIndexInvalid) {
		// 对象不可扩展，不能添加新属性
		if (!tag_.is_extensible_) {
			return;
		}
	}

	// 支持只定义 getter 或只定义 setter，或两者都定义
	if (getter && setter) {
		// 同时定义 getter 和 setter
		// QuickJS 中这会创建一个包含两个函数的 accessor 属性
		// 但我们的实现需要两个独立的属性，使用特殊的命名约定
		// 或者我们可以使用一个属性，但需要修改 ShapeProperty 来支持两个函数

		// 当前实现：分别存储 getter 和 setter
		uint32_t getter_flags = flags | ShapeProperty::kIsGetter;
		uint32_t setter_flags = flags | ShapeProperty::kIsSetter;

		SetPropertyWithFlags(context, key, Value(getter), getter_flags);
		// 注意：这里 setter 会覆盖 getter，需要更好的设计
		// TODO: 改进存储结构，支持同时存储 getter 和 setter
	}
	else if (getter) {
		// 只定义 getter（只读属性）
		uint32_t getter_flags = flags | ShapeProperty::kIsGetter;
		SetPropertyWithFlags(context, key, Value(getter), getter_flags);
	}
	else if (setter) {
		// 只定义 setter（只写属性）
		uint32_t setter_flags = flags | ShapeProperty::kIsSetter;
		SetPropertyWithFlags(context, key, Value(setter), setter_flags);
	}
	// else: 既没有 getter 也没有 setter，不做任何操作
}

bool Object::GetProperty(Context* context, ConstIndex key, Value* value) {
	// 如果配置了exotic，需要先查找(也就是class_def中的GetProperty等方法)

	// 1. 查找自身属性
	auto index = shape_->Find(key);
	if (index != kPropertySlotIndexInvalid) {
		uint32_t prop_flags = GetPropertyFlags(index);

		// 检查是否是 getter
		if (prop_flags & ShapeProperty::kIsGetter) {
			// 自动调用 getter，this 绑定到当前对象，无参数
			// 这符合 JavaScript 规范：访问 getter 属性时自动调用
			*value = context->CallFunction(&GetPropertyValue(index), Value(this), static_cast<Value*>(nullptr), static_cast<Value*>(nullptr));
			return true;
		}

		// 检查是否是 setter（只有 setter 的属性，读取时返回 undefined）
		if (prop_flags & ShapeProperty::kIsSetter) {
			*value = Value();  // 返回 undefined
			return true;
		}

		// 普通数据属性，直接返回值
		*value = GetPropertyValue(index);
		return true;
	}

	// 2. 原型链查找
	auto& prototype = GetPrototype(&context->runtime());
	if (prototype.IsObject()) {
		auto success = prototype.object().GetProperty(context, key, value);
		if (success) return true;
	}

	return false;
}

bool Object::HasProperty(Context* context, ConstIndex key) {
	Value value;
	return GetProperty(context, key, &value);
}

void Object::DelProperty(Context* context, ConstIndex key) {
	// 检查属性是否存在且可配置
	auto index = shape_->Find(key);
	if (index == kPropertySlotIndexInvalid) {
		// 属性不存在,无需删除
		return;
	}

	uint32_t prop_flags = GetPropertyFlags(index);
	// 只有可配置的属性才能被删除
	if (!(prop_flags & ShapeProperty::kConfigurable)) {
		return;
	}

	// TODO: 实现属性删除逻辑
	throw std::runtime_error("DelProperty error.");
}

void Object::SetComputedProperty(Context* context, const Value& key, Value&& val) {
	auto idx = context->FindConstOrInsertToLocal(key);
	return SetProperty(context, idx, std::move(val));
}

bool Object::GetComputedProperty(Context* context, const Value& key, Value* value) {
	auto idx = context->FindConstOrInsertToLocal(key);
	return GetProperty(context, idx, value);
}

void Object::DelComputedProperty(Context* context, const Value& key) {
	auto idx = context->FindConstOrInsertToLocal(key);
	return DelProperty(context, key.const_index());
}

Value Object::ToString(Context* context) {
	std::string str = "{";

	for (auto& slot : properties_) {
		Value value = slot.value;
		// str += context->runtime().const_pool()[prop.first].string().data();
		// str += ":";
		if (value.IsObject() && &value.object() == this) {
			str += "self";
		}
		else {
			str += value.ToString(context).string_view();
		}
		str += ",";
	}
	str.pop_back();

	str += "}";
	return Value(String::New(str));
}

const Value& Object::GetPrototype(Runtime* runtime) const {
	// 未来不存class_id，通过shape获取prototype
	return GetClassDef(runtime).prototype();
}

ClassDef& Object::GetClassDef(Runtime* runtime) const {
	return runtime->class_def_table().at(class_id());
}


void Object::Reference() {
	++tag_.ref_count_;
}

void Object::Dereference() {
	// 对象正在gc过程中，其属性中可能包含对象自身，通过gc_mark避免重复析构
	if (!gc_mark()) {
		WeakDereference();
		if (tag_.ref_count_ == 0) {
			delete this;
		}
	}
}

void Object::WeakDereference() {
	assert(tag_.ref_count_ > 0);
	--tag_.ref_count_;
}

void Object::Freeze() {
	// 1. 标记为不可扩展
	tag_.is_extensible_ = 0;

	// 2. 将所有现有属性的标志设置为不可写和不可配置
	// 现在每个对象都有独立的属性标志，可以安全修改
	for (uint32_t i = 0; i < properties_.size(); i++) {
		uint32_t flags = properties_[i].flags;

		// 只修改数据属性，保留 accessor 的特殊标志
		if (!(flags & (ShapeProperty::kIsGetter | ShapeProperty::kIsSetter))) {
			// 移除 writable 和 configurable 标志
			properties_[i].flags = flags & ~(ShapeProperty::kWritable | ShapeProperty::kConfigurable);
		} else {
			// Accessor 属性只移除 configurable
			properties_[i].flags = flags & ~ShapeProperty::kConfigurable;
		}
	}

	// 3. 标记为已冻结
	tag_.is_frozen_ = 1;
}

bool Object::IsFrozen() const {
	// 冻结状态由对象级别标志决定
	return tag_.is_frozen_;
}

void Object::Seal() {
	// 1. 标记为不可扩展
	tag_.is_extensible_ = 0;

	// 2. 将所有现有属性设置为不可配置
	for (uint32_t i = 0; i < properties_.size(); i++) {
		properties_[i].flags &= ~ShapeProperty::kConfigurable;
	}

	// 3. 标记为已密封
	tag_.is_sealed_ = 1;
}

bool Object::IsSealed() const {
	// 密封状态由对象级别标志决定
	return tag_.is_sealed_;
}

void Object::PreventExtensions() {
	// 只需要标记为不可扩展
	tag_.is_extensible_ = 0;
}

bool Object::IsExtensible() const {
	return tag_.is_extensible_;
}

} // namespace mjs
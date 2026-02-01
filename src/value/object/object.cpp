#include <mjs/value/object/object.h>

#include <functional>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/shape/shape.h>
#include <mjs/const_index_embedded.h>
#include <mjs/class_def/object_class_def.h>

namespace mjs {

Object::Object(Context* context, ClassId class_id, GCObjectType gc_type)
	: GCObject(gc_type, sizeof(Object))
{
	tag_.is_extensible_ = 1;  // 默认可扩展

	tag_.class_id_ = static_cast<uint16_t>(class_id);

	shape_ = &context->shape_manager().empty_shape();

	shape_->Reference();
}

Object::~Object() {
	// 对于 GC 管理的对象，析构函数由 GC 系统在清理时调用
	// 此时不需要检查引用计数
	shape_->Dereference();
}

void Object::GCTraverse(Context* context, GCTraverseCallback callback) {
	// 遍历所有属性
	for (auto& slot : properties_) {
		callback(context, slot.value);
	}

	// 遍历原型对象
	if (tag_.set_proto_) {
		auto index = shape_->Find(ConstIndexEmbedded::kProto);
		if (index != kPropertySlotIndexInvalid) {
			callback(context, properties_[index].value);
		}
	}
}

bool Object::GetProperty(Context* context, ConstIndex key, Value* value) {
	// 如果配置了exotic，需要先查找(也就是class_def中的GetProperty等方法)

	if (key == ConstIndexEmbedded::kProto) {
		*value = GetPrototype(context);
		return true;
	}

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
	auto& prototype = GetPrototype(context);
	if (prototype.IsObject()) {
		auto success = prototype.object().GetProperty(context, key, value);
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

bool Object::HasProperty(Context* context, ConstIndex key) {
	Value value;
	return GetProperty(context, key, &value);
}

bool Object::DelProperty(Context* context, ConstIndex key, Value* value) {
	// 检查属性是否存在且可配置
	auto index = shape_->Find(key);
	if (index == kPropertySlotIndexInvalid) {
		// 属性不存在,无需删除
		return false;
	}

	uint32_t prop_flags = GetPropertyFlags(index);
	// 只有可配置的属性才能被删除
	if (!(prop_flags & ShapeProperty::kConfigurable)) {
		return false;
	}

	// TODO: 实现属性删除逻辑
	throw InternalError("DelProperty error.");

	*value = GetPropertyValue(index);
	return true;
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

void Object::SetComputedProperty(Context* context, const Value& key, Value&& val) {
	auto idx = context->FindConstOrInsertToLocal(key.ToString(context));
	return SetProperty(context, idx, std::move(val));
}

bool Object::GetComputedProperty(Context* context, const Value& key, Value* value) {
	auto idx = context->FindConstOrInsertToLocal(key.ToString(context));
	return GetProperty(context, idx, value);
}

bool Object::DelComputedProperty(Context* context, const Value& key, Value* value) {
	auto idx = context->FindConstOrInsertToLocal(key.ToString(context));
	return DelProperty(context, key.const_index(), value);
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

const Value& Object::GetPrototype(Context* context) const {
	if (tag_.set_proto_) {
		auto index = shape_->Find(ConstIndexEmbedded::kProto);
		assert(index != kPropertySlotIndexInvalid);
		return properties_[index].value;
	}
	return context->runtime().class_def_table()[static_cast<ClassId>(tag_.class_id_)].prototype();
}

void Object::SetPrototype(Context* context, Value prototype) {
	if (!tag_.set_proto_) {
		tag_.set_proto_ = true;
	}
	SetProperty(context, ConstIndexEmbedded::kProto, std::move(prototype));
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

Object* Object::New(Context* context) {
	return New(context, GCObjectType::kObject);
}

Object* Object::New(Context* context, GCObjectType gc_type) {
	// 使用 GCHeap 分配内存
	GCHeap* heap = context->gc_manager().heap();

	// 计算需要分配的总大小（Object 已经包含 GCObject 子对象）
	size_t total_size = sizeof(Object);

	// 分配原始内存，不构造 GCObject
	// Object 构造函数会初始化 GCObject 基类部分
	void* mem = heap->AllocateRaw(gc_type, total_size);
	if (!mem) {
		return nullptr;
	}

	// 使用 placement new 在分配的内存中构造 Object
	// 这会先构造 GCObject 基类，然后构造 Object 派生类
	Object* obj = new (mem) Object(context, ClassId::kObject, gc_type);

	return obj;
}

} // namespace mjs
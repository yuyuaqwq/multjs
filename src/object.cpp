#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/shape.h>

namespace mjs {

Object::Object(Runtime* runtime, ClassId class_id) {
	// 挂入gc链表
	runtime->gc_manager().AddObject(this);

	tag_.class_id_ = static_cast<uint16_t>(class_id);
	shape_ = &runtime->shape_manager().empty_shape();

	// runtime的对象，只允许在初始化阶段变更，因此引用和解引用不考虑线程安全问题
	shape_->Reference();
}

Object::Object(Context* context, ClassId class_id) {
	// 挂入gc链表
	context->gc_manager().AddObject(this);

	shape_ = &context->shape_manager().empty_shape();
	shape_->Reference();

	tag_.class_id_ = static_cast<uint16_t>(class_id);
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
	if (index != kShapeSlotIndexInvalid) {
		*value = values_[index];
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
	assert(!context || !tag_.is_const_);

	// 检查是否已经存在该属性
	auto index = shape_->Find(key);
	if (index != kShapeSlotIndexInvalid) {
		auto& property = shape_->GetProperty(index);

		// 情况 1：属性是 setter
		if (property.is_setter()) {
			// 自动调用 setter，this 绑定到当前对象，value 作为参数
			Value argv[] = { std::move(value) };
			context->CallFunction(&values_[index], Value(this), std::begin(argv), std::end(argv));
			return;
		}

		// 情况 2：属性是 getter（只读访问器）
		if (property.is_getter()) {
			// 尝试写入只有getter的属性应该失败
			// 但这里我们选择静默忽略，因为有些场景可能需要这种语义
			// 可以根据需要改为抛出异常
			return;
		}

		// 情况 3：普通数据属性
		// 检查是否可写
		if (!property.is_writable()) {
			// 只读属性，静默忽略（或者可以抛出 TypeError）
			return;
		}

		// 可写的普通属性，直接更新值
		values_[index] = std::move(value);
		return;
	}

	// 新属性，使用默认标志（包含 enumerable, configurable, writable）
	uint32_t default_flags = ShapeProperty::kDefault;
	index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(default_flags, key));
	if (index < values_.size()) {
		values_[index] = std::move(value);
	}
	else {
		assert(index == values_.size());
		values_.emplace_back(std::move(value));
	}
}

void Object::SetPropertyWithFlags(Context* context, ConstIndex key, Value&& value, uint32_t flags) {
	assert(!context || !tag_.is_const_);
	auto index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(flags, key));
	if (index < values_.size()) {
		values_[index] = std::move(value);
	}
	else {
		assert(index == values_.size());
		values_.emplace_back(std::move(value));
	}
}

void Object::DefineAccessorProperty(Context* context, ConstIndex key,
                                     FunctionObject* getter,
                                     FunctionObject* setter,
                                     uint32_t flags) {
	assert(!context || !tag_.is_const_);

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
	if (index != kShapeSlotIndexInvalid) {
		auto& property = shape_->GetProperty(index);

		// 检查是否是 getter
		if (property.is_getter()) {
			// 自动调用 getter，this 绑定到当前对象，无参数
			// 这符合 JavaScript 规范：访问 getter 属性时自动调用
			*value = context->CallFunction(&values_[index], Value(this), static_cast<Value*>(nullptr), static_cast<Value*>(nullptr));
			return true;
		}

		// 检查是否是 setter（只有 setter 的属性，读取时返回 undefined）
		if (property.is_setter()) {
			*value = Value();  // 返回 undefined
			return true;
		}

		// 普通数据属性，直接返回值
		*value = values_[index];
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
	assert(!tag_.is_const_);
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

	for (auto value : values_) {
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

} // namespace mjs
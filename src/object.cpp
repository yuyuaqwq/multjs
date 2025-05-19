#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

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


void Object::SetProperty(Runtime* runtime, std::string_view key, Value&& value) {
	auto const_index = runtime->const_pool().insert(mjs::Value(key));
	SetProperty(nullptr, const_index, std::move(value));
}

bool Object::GetProperty(Runtime* runtime, std::string_view key, Value* value) {
	auto const_index = runtime->const_pool().insert(mjs::Value(key));
	return GetProperty(nullptr, const_index, value);
}


void Object::SetProperty(Context* context, ConstIndex key, Value&& value) {
	assert(!context || !tag_.is_const_);
	auto index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(0, key));
	if (index < values_.size()) {
		values_[index] = std::move(value);
	}
	else {
		assert(index == values_.size());
		values_.emplace_back(std::move(value));
	}
}

bool Object::GetProperty(Context* context, ConstIndex key, Value* value) {
	// 如果配置了exotic，需要先查找(也就是class_def中的GetProperty等方法)

	// 1. 查找自身属性
	auto index = shape_->Find(key);
	if (index != -1) {
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
	auto& class_def = runtime->class_def_table().at(class_id());
	return class_def.prototype();
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
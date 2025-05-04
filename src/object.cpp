#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

Object::Object(Runtime* runtime) {
	// 仅runtime使用的对象就提前new
	property_map_ = new PropertyMap(runtime);
}

Object::Object(Context* context) {
	// 挂入context obj链表
	if (context) {
		context->AddObject(this);
	}
	else {
		tag_.is_const_ = true;
	}
}

Object::~Object() {
	assert(gc_mark() || tag_.ref_count_ == 0);
	if (property_map_) {
		delete property_map_;
	}
	unlink();
}

void Object::SetProperty(Context* context, ConstIndex key, Value&& value) {
	assert(!context || !tag_.is_const_);
	if (!property_map_) property_map_ = new PropertyMap(context);
	property_map_->set(context, key, std::move(value));
}

bool Object::GetProperty(Context* context, ConstIndex key, Value* value) {
	// 1. 查找自身属性
	if (property_map_) {
		auto iter = property_map_->find(key);
		if (iter != property_map_->end()) {
			*value = iter->second;
			return true;
		}
	}
		
	// 2. class def查找
	auto& class_def = context->runtime().class_def_table().at(class_id());
	auto success = class_def.GetProperty(context, this, key, value);
	if (success) return true;
	
	return false;
}

void Object::DelProperty(Context* context, ConstIndex key) {
	assert(!tag_.is_const_);
	if (!property_map_) return;
	property_map_->erase(context, key);
}

void Object::SetComputedProperty(Context* context, const Value& key, Value&& val) {
	auto idx = key.const_index();
	if (idx.is_invalid()) {
		idx = context->const_pool().insert(key);
	}
	return SetProperty(context, idx, std::move(val));
}

bool Object::GetComputedProperty(Context* context, const Value& key, Value* value) {
	auto idx = key.const_index();
	if (key.const_index().is_invalid()) {
		idx = context->const_pool().insert(key);
	}
	return GetProperty(context, idx, value);
}

void Object::DelComputedProperty(Context* context, const Value& key) {
	auto idx = key.const_index();
	if (key.const_index().is_invalid()) {
		idx = context->const_pool().insert(key);
	}
	return DelProperty(context, key.const_index());
}

Value Object::ToString(Context* context) {
	std::string str = "{";
	if (property_map_) {
		for (auto prop : *property_map_) {
			str += context->runtime().const_pool()[prop.first].string().data();
			str += ":";
			str += prop.second.ToString(context).string_view();
			str += ",";
		}
		str.pop_back();
	}
	str += "}";
	return Value(String::make(str));
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
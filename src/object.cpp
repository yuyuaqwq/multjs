#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

Object::Object(Context* context) {
	if (tag_.ref_count_ == 0) {
		// 挂入context obj链表
		context->AddObject(this);
	}
}

Object::~Object() {
	assert(tag_.ref_count_ == 0);
	if (property_map_) {
		delete property_map_;
	}
	unlink();
}

void Object::SetProperty(Context* context, const Value& key, Value&& val) {
	if (!property_map_) property_map_ = new PropertyMap();
	(*property_map_)[key] = std::move(val);
}

Value* Object::GetProperty(Context* context, const Value& key) {
	// 1. 查找自身属性
	if (property_map_) {
		auto iter = property_map_->find(key);
		if (iter != property_map_->end()) {
			return &iter->second;
		}
	}
		
	// 2. class def查找
	auto& class_def = context->runtime().class_def_table().at(class_id());
	auto val = class_def.GetProperty(&context->runtime(), key);
	if (val) return val;

	// 3. 原型链查找
	if (prototype_.IsObject()) {
		return prototype_.object().GetProperty(context, key);
	}
	return nullptr;
}

void Object::DelProperty(Context* context, const Value& key) {
	if (!property_map_) return;
	property_map_->erase(key);
}

void Object::Reference() {
	++tag_.ref_count_;
}

void Object::Dereference() {
	--tag_.ref_count_;
	if (tag_.ref_count_ == 0) {
		delete this;
	}
}

void Object::WeakDereference() {
	--tag_.ref_count_;
}

} // namespace mjs
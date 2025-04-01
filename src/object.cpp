#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

void Object::SetProperty(Runtime* runtime, const Value& key, Value&& val) {
	if (!property_map_) property_map_ = new PropertyMap();
	(*property_map_)[key] = std::move(val);
}

Value* Object::GetProperty(Runtime* runtime, const Value& key) {
	// 1. ������������
	if (property_map_) {
		auto iter = property_map_->find(key);
		if (iter != property_map_->end()) {
			return &iter->second;
		}
	}
		
	// 2. class����
	auto& class_def = runtime->class_def_table().at(class_id());
	auto iter = class_def.property_map().find(key);
	if (iter != class_def.property_map().end()) {
		return &iter->second;
	}

	// 3. ԭ��������
	if (prototype_.IsObject()) {
		return prototype_.object().GetProperty(runtime, key);
	}
	return nullptr;
}

void Object::DelProperty(Context* context, const Value& key) {
	if (!property_map_) return;
	property_map_->erase(key);
}

} // namespace mjs
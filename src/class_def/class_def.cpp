#include <mjs/class_def/class_def.h>

#include <mjs/context.h>

namespace mjs {

void ClassDef::SetProperty(Context* context, Object* obj, ConstIndex key, Value&& val) {
	property_map_.set(&context->runtime(), key, std::move(val));
}

bool ClassDef::GetProperty(Context* context, Object* obj, ConstIndex key, Value* value) {
	auto iter = property_map_.find(key);
	if (iter != property_map_.end()) {
		*value = iter->second;
		return true;
	}
	return false;
}

bool ClassDef::HasProperty(Context* context, Object* obj, ConstIndex key) {
	auto iter = property_map_.find(key);
	return iter != property_map_.end();
}

bool ClassDef::DelProperty(Context* context, Object* obj, ConstIndex key) {
	auto res = property_map_.erase(&context->runtime(), key);
	return res > 0;
}

void ClassDef::SetStaticProperty(Runtime* runtime, ConstIndex key, Value&& val) {
	static_property_map_.set(runtime, key, std::move(val));
}

bool ClassDef::GetStaticProperty(Runtime* runtime, ConstIndex key, Value* value) {
	auto iter = static_property_map_.find(key);
	if (iter != static_property_map_.end()) {
		*value = iter->second;
		return false;
	}
	return true;
}

bool ClassDef::HasStaticProperty(Runtime* runtime, ConstIndex key) {
	auto iter = static_property_map_.find(key);
	return iter != static_property_map_.end();
}

void ClassDef::DelStaticProperty(Runtime* runtime, ConstIndex key) {
	static_property_map_.erase(runtime, key);
}


} // namespace msj
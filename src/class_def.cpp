#include <mjs/class_def.h>

#include <mjs/context.h>
#include <mjs/object_impl/constructor_object.h>

namespace mjs {

ClassDef::ClassDef(Runtime* runtime, ClassId id, std::string name)
	: id_(id)
{
	name_string_ = std::move(name);
	name_ = runtime->const_pool().insert(Value(name_string_));
	constructor_object_ = Value(new ConstructorObject(runtime, id_));
	prototype_ = Value(new Object(runtime, ClassId::kObject));

	auto prototype = prototype_;
	constructor_object_.object().SetProperty(runtime, "prototype", std::move(prototype));

	// 挂载构造函数对象到全局对象
	auto constructor_object = constructor_object_;
	runtime->global_this().object().SetProperty(nullptr, name_, std::move(constructor_object));
}



void ClassDef::SetProperty(Context* context, Object* obj, ConstIndex key, Value&& val) {
	//property_map_.set(&context->runtime(), key, std::move(val));
}

bool ClassDef::GetProperty(Context* context, Object* obj, ConstIndex key, Value* value) {
	//auto iter = property_map_.find(key);
	//if (iter != property_map_.end()) {
	//	*value = iter->second;
	//	return true;
	//}
	return false;
}

bool ClassDef::HasProperty(Context* context, Object* obj, ConstIndex key) {
	//auto iter = property_map_.find(key);
	//return iter != property_map_.end();
	return false;
}

bool ClassDef::DelProperty(Context* context, Object* obj, ConstIndex key) {
	//auto res = property_map_.erase(&context->runtime(), key);
	//return res > 0;
	return false;
}


} // namespace msj
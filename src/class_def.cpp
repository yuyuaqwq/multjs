#include <mjs/class_def.h>

#include <mjs/context.h>
#include <mjs/object_impl/constructor_object.h>

namespace mjs {

ClassDef::ClassDef(Runtime* runtime, ClassId id, const char* name)
	: id_(id)
{
	name_ = runtime->const_pool().insert(Value(name));
	constructor_object_ = Value(ConstructorObject::New(runtime, id_));
	prototype_ = Value(Object::New(runtime));

	auto prototype = prototype_;
	constructor_object_.object().SetProperty(runtime, "prototype", std::move(prototype));

	// 挂载构造函数对象到全局对象
	auto constructor_object = constructor_object_;
	runtime->global_this().object().SetProperty(nullptr, name_, std::move(constructor_object));
}

ClassDef::~ClassDef() {}

} // namespace msj
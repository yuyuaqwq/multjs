#include <mjs/class_def.h>

#include <mjs/context.h>
#include <mjs/object_impl/constructor_object.h>

namespace mjs {

ClassDef::ClassDef(Runtime* runtime, ClassId id, const char* name)
	: id_(id)
	, name_string_(name)
{
	name_ = runtime->global_const_pool().insert(Value(name));
	constructor_object_ = Value(ConstructorObject::New(runtime, id_));
	prototype_ = Value(Object::New(runtime));

	// Debug: 检查prototype_是否正确初始化
	// printf("ClassDef %s: prototype_.type() = %d, IsObject() = %d\n",
	//        name, static_cast<int>(prototype_.type()), prototype_.IsObject());

	auto prototype = prototype_;
	constructor_object_.object().SetProperty(runtime, runtime->key_const_index_table().prototype_const_index(), std::move(prototype));

	// 挂载构造函数对象到全局对象
	auto constructor_object = constructor_object_;
	runtime->global_this().object().SetProperty(runtime, name_, std::move(constructor_object));
}

ClassDef::~ClassDef() {}

} // namespace msj
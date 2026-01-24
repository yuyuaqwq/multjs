#include <mjs/class_def.h>

#include <mjs/context.h>
#include <mjs/object_impl/constructor_object.h>

namespace mjs {

ClassDef::ClassDef(Runtime* runtime, ClassId id, const char* name)
	: id_(id)
	, name_string_(name)
{
	name_ = runtime->global_const_pool().Insert(Value(name));

	// Debug: 检查prototype_是否正确初始化
	// printf("ClassDef %s: prototype_.type() = %d, IsObject() = %d\n",
	//        name, static_cast<int>(prototype_.type()), prototype_.IsObject());

	prototype_ = Value(Object::New(runtime));
	constructor_ = Value(ConstructorObject::New(runtime, id_));
	auto prototype_const_index = runtime->global_const_pool().Insert(Value("prototype"));
	auto constructor_const_index = runtime->global_const_pool().Insert(Value("constructor"));

	auto prototype = prototype_;
	constructor_.object().SetProperty(runtime, prototype_const_index, std::move(prototype));

	// 挂载构造函数到全局对象
	Value constructor = constructor_;
	runtime->global_this().object().SetProperty(runtime, name_, std::move(constructor));
		
	constructor = constructor_;
	prototype_.object().SetProperty(runtime, constructor_const_index, std::move(constructor));
}

ClassDef::~ClassDef() {}

} // namespace msj
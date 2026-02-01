#include <mjs/class_def/class_def.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/object/constructor_object.h>

namespace mjs {

ClassDef::ClassDef(Runtime* runtime, ClassId id, const char* name)
	: id_(id)
{
	if (name) {
		name_ = runtime->global_const_pool().FindOrInsert(Value(name));
		name_string_ = name;
	}
	else {
		name_ = kConstIndexInvalid;
	}
	
	// Debug: 检查prototype_是否正确初始化
	// printf("ClassDef %s: prototype_.type() = %d, IsObject() = %d\n",
	//        name, static_cast<int>(prototype_.type()), prototype_.IsObject());

	prototype_ = Value(Object::New(&runtime->default_context()));

	if (name_ != kConstIndexInvalid) {
		constructor_ = Value(ConstructorObject::New(&runtime->default_context(), id_));

		auto prototype = prototype_;
		constructor_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kPrototype, std::move(prototype));

		// 挂载构造函数到全局对象
		Value constructor = constructor_;
		runtime->global_this().object().SetProperty(&runtime->default_context(), name_, std::move(constructor));

		constructor = constructor_;
		prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kConstructor, std::move(constructor));
	}
}

ClassDef::~ClassDef() {}

} // namespace msj
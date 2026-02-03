#include <mjs/class_def/class_def.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
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

	GCHandleScope<2> scope(&runtime->default_context());
	auto prototype_obj = scope.New<Object>();
	prototype_ = prototype_obj.ToValue();

	if (name_ != kConstIndexInvalid) {
		auto constructor_obj = scope.New<ConstructorObject>(id_);
		constructor_ = constructor_obj.ToValue();

		// 挂载prototype到构造函数对象
		constructor_obj->SetProperty(&runtime->default_context(), ConstIndexEmbedded::kPrototype, Value(prototype_));

		// 挂载构造函数到全局对象
		runtime->global_this().object().SetProperty(&runtime->default_context(), name_, Value(constructor_));

		prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kConstructor, Value(constructor_));
	}
}

ClassDef::~ClassDef() {}

} // namespace msj
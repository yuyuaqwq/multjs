#include <mjs/class_def_impl/function_object_class_def.h>
#include <mjs/context.h>

namespace mjs {

FunctionObjectClassDef::FunctionObjectClassDef(Runtime* runtime)
    : ClassDef(runtime, ClassId::kFunctionObject, "Function")
{
	prototype_const_index_ = runtime->global_const_pool().insert(Value("prototype"));
	constructor_const_index_ = runtime->global_const_pool().insert(Value("constructor"));
}

} // namespace mjs

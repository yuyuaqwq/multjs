#include <mjs/object_impl/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(Context* context, FunctionDef* function_def) noexcept
	: Object(context, ClassId::kFunctionObject)
	, function_def_(function_def)
{
	closure_env_.closure_var_refs().resize(function_def->closure_var_defs().size());
}

} // namespace mjs
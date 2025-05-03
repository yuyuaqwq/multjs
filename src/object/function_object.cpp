#include <mjs/object/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(Context* context, FunctionDef* function_def) noexcept
	: Object(context)
	, function_def_(function_def)
{
	closure_env_.resize(function_def->closure_var_defs().size());
}

} // namespace mjs
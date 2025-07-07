#include <mjs/object_impl/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(Context* context, FunctionDefBase* function_def) noexcept
	: FunctionObject(context, function_def, ClassId::kFunctionObject) {}

FunctionObject::FunctionObject(Context* context, FunctionDefBase* function_def, ClassId class_id) noexcept
	: Object(context, class_id)
	, function_def_(function_def)
{
	closure_env_.closure_var_refs().resize(function_def->closure_var_table().closure_var_defs().size());
}
} // namespace mjs
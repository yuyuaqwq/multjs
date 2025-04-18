#include <mjs/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(Context* context, FunctionDef* function_def) noexcept
	: Object(context)
	, function_def_(function_def) {}

} // namespace mjs
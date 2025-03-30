#include <mjs/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(FunctionDef* function_def) noexcept
	: function_def_(function_def) {}

} // namespace mjs
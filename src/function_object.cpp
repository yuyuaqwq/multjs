#include <mjs/function_object.h>

namespace mjs {

FunctionObject::FunctionObject(FunctionDef* func_def) noexcept
	: func_def_(func_def) {}

} // namespace mjs
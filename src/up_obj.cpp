#include "up_obj.h"

namespace mjs {

UpValueObject::UpValueObject(uint32_t index, FunctionBodyObject* func_body) noexcept
	: index(index)
	, func_body(func_body) {}

} // namespace mjs
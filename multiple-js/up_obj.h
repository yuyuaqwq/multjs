#include <cstdint>

#include "object.h"

namespace mjs {

class FunctionBodyObject;
class UpValueObject : public Object {
public:
	UpValueObject(uint32_t t_index, FunctionBodyObject* func_body) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_body;
};

} // namespace mjs
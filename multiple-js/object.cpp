#include "object.h"

namespace mjs {


FunctionBodyObject::FunctionBodyObject(uint32_t par_count) noexcept
	: par_count(par_count) {}

std::string FunctionBodyObject::Disassembly() {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code.Size(); ) {
		str += byte_code.Disassembly(pc) + "\n";
	}
	return str;
}


UpValueObject::UpValueObject(uint32_t index, FunctionBodyObject* func_body) noexcept
	: index(index)
	, func_body(func_body) {}

} // namespace mjs
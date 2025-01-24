#include <mjs/func_obj.h>

namespace mjs {

FunctionBodyObject::FunctionBodyObject(FunctionBodyObject* parent, uint32_t par_count) noexcept
	: parent(parent)
	, par_count(par_count) {}

std::string FunctionBodyObject::Disassembly() {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code.Size(); ) {
		str += byte_code.Disassembly(pc) + "\n";
	}
	return str;
}

FunctionRefObject::FunctionRefObject(FunctionBodyObject* func_body) noexcept
	: func_body_(func_body) {}


} // namespace mjs
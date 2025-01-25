#include <mjs/func_obj.h>

namespace mjs {

FunctionDefObject::FunctionDefObject(uint32_t par_count) noexcept
	: par_count(par_count) {}

std::string FunctionDefObject::Disassembly() {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code.Size(); ) {
		str += byte_code.Disassembly(pc) + "\n";
	}
	return str;
}

FunctionObject::FunctionObject(FunctionDefObject* func_def) noexcept
	: func_def_(func_def) {}


} // namespace mjs
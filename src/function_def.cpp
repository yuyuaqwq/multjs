#include <mjs/function_def.h>

namespace mjs {

FunctionDef::FunctionDef(uint32_t par_count) noexcept
	: par_count(par_count) {}

std::string FunctionDef::Disassembly() {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code.Size(); ) {
		str += byte_code.Disassembly(pc) + "\n";
	}
	return str;
}

} // namespace mjs
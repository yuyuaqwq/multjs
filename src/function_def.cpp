#include <mjs/function_def.h>

namespace mjs {

FunctionDef::FunctionDef(uint32_t par_count) noexcept
	: par_count_(par_count) {}

std::string FunctionDef::Disassembly(Context* context) {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code_.Size(); ) {
        OpcodeType opcode;
        uint32_t par;

		str += byte_code_.Disassembly(context, pc, opcode, par, this);


        str += "\n";
	}
	return str;
}

} // namespace mjs
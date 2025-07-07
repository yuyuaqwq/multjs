#include <mjs/function_def.h>

#include <mjs/runtime.h>

namespace mjs {

	FunctionDefBase::FunctionDefBase(ModuleDef* module_def, std::string name, uint32_t par_count) noexcept
	: module_def_(module_def)
	, name_(name)
	, par_count_(par_count) {}

std::string FunctionDefBase::Disassembly(Context* context) const {
	std::string str;
	for (uint32_t pc = 0; pc < bytecode_table().Size(); ) {
        OpcodeType opcode;
        uint32_t par;

		str += bytecode_table().Disassembly(context, pc, opcode, par, this);

        str += "\n";
	}
	return str;
}

} // namespace mjs
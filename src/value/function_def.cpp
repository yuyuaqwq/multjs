#include <mjs/value/function_def.h>

#include <mjs/runtime.h>

#ifdef ENABLE_JIT
#include <mjs/jit/hotness_counter.h>
#endif

namespace mjs {

	FunctionDefBase::FunctionDefBase(ModuleDef* module_def, std::string name, uint32_t param_count) noexcept
	: module_def_(module_def)
	, name_(std::move(name))
	, param_count_(param_count)
#ifdef ENABLE_JIT
	, hotness_counter_(std::make_unique<jit::HotnessCounter>())
#endif
	{}

std::string FunctionDefBase::Disassembly(Context* context) const {
	std::string str;
	for (uint32_t pc = 0; pc < bytecode_table().Size(); ) {
        OpcodeType opcode;
        uint32_t param;

		str += bytecode_table().Disassembly(context, pc, opcode, param, this);

        str += "\n";
	}
	return str;
}

} // namespace mjs
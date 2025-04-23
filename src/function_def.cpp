#include <mjs/function_def.h>

#include <mjs/runtime.h>

namespace mjs {

FunctionDef::FunctionDef(Runtime* runtime, std::string name, uint32_t par_count) noexcept
	: runtime_(runtime)
	, name_(name)
	, par_count_(par_count) {}

std::string FunctionDef::Disassembly(Context* context) const {
	std::string str;
	for (uint32_t pc = 0; pc < byte_code_.Size(); ) {
        OpcodeType opcode;
        uint32_t par;

		str += byte_code_.Disassembly(context, pc, opcode, par, this);

        str += "\n";
	}
	return str;
}

void FunctionDef::AddExportVar(std::string name, VarIndex var_idx) {
	export_var_defs_.emplace(runtime_->const_pool().insert(Value(name)), var_idx);
	AddClosureVar(var_idx, std::nullopt);
}

} // namespace mjs
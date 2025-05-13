#pragma once

#include <mjs/function_def.h>

namespace mjs {

class Runtime;
// 不会有循环引用问题，仅使用引用计数管理
class ModuleDef : public FunctionDef {
public:
	ModuleDef(Runtime* runtime, std::string name, uint32_t par_count)
		: FunctionDef(runtime, name, par_count) {}

	void AddExportVar(std::string name, VarIndex var_idx);

	const auto& export_var_defs() const { return export_var_defs_; }
	auto& export_var_defs() { return export_var_defs_; }

private:
	struct ExportVarDef {
		uint32_t export_var_index;
		VarIndex var_index;
	};
	std::unordered_map<std::string, ExportVarDef> export_var_defs_;
};

} // namespace mjs


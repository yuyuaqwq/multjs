#pragma once

#include <mjs/function_def.h>
#include <mjs/source.h>

namespace mjs {

class Runtime;
// 不会有循环引用问题，仅使用引用计数管理
class ModuleDef : public FunctionDef {
public:
	ModuleDef(Runtime* runtime, std::string name, std::string_view source, uint32_t par_count)
		: runtime_(runtime)
		, FunctionDef(this, name, par_count)
	{
		line_table_.Build(source);
	}

	void AddExportVar(std::string name, VarIndex var_idx);

	const auto& export_var_defs() const { return export_var_defs_; }
	auto& export_var_defs() { return export_var_defs_; }

	const auto& line_table() const { return line_table_; }

private:
	Runtime* runtime_;

	struct ExportVarDef {
		uint32_t export_var_index;
		VarIndex var_index;
	};
	std::unordered_map<std::string, ExportVarDef> export_var_defs_;

	LineTable line_table_;
};

} // namespace mjs


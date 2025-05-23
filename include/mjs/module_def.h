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

	const auto& export_var_def_table() const { return export_var_def_table_; }
	auto& export_var_def_table() { return export_var_def_table_; }

	const auto& line_table() const { return line_table_; }

private:
	Runtime* runtime_;

	ExportVarDefTable export_var_def_table_;
	LineTable line_table_;
};

} // namespace mjs


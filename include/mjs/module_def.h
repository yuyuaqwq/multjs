#pragma once

#include <mjs/function_def.h>
#include <mjs/source.h>

namespace mjs {

class Runtime;
// 不会有循环引用问题，仅使用引用计数管理
class ModuleDef : public ReferenceCounter<ModuleDef>, public FunctionDefBase {
public:
	static ModuleDef* New(Runtime* runtime, std::string name, std::string_view source, uint32_t param_count) {
		return new ModuleDef(runtime, std::move(name), source, param_count);
	}

	const auto& export_var_def_table() const { return export_var_def_table_; }
	auto& export_var_def_table() { return export_var_def_table_; }

	const auto& line_table() const { return line_table_; }

private:
	ModuleDef(Runtime* runtime, std::string name, std::string_view source, uint32_t param_count)
		: runtime_(runtime)
		, FunctionDefBase(this, name, param_count)
	{
		line_table_.Build(source);
	}

private:
	Runtime* runtime_;

	ExportVarDefTable export_var_def_table_;
	LineTable line_table_;
};

} // namespace mjs


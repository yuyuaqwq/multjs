#include <mjs/module_def.h>

#include <mjs/runtime.h>

namespace mjs {

void ModuleDef::AddExportVar(std::string_view name, VarIndex var_idx) {
	auto res = export_var_defs_.emplace(runtime_->const_pool().insert(Value(String::make(name))),
		ExportVarDef{
			.export_var_index = uint32_t(export_var_defs_.size()),
			.var_index = var_idx,
		}
	);
	assert(res.second);
}

} // namespace mjs
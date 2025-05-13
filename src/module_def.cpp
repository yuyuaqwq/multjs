#include <mjs/module_def.h>

#include <mjs/runtime.h>

namespace mjs {

void ModuleDef::AddExportVar(std::string name, VarIndex var_idx) {
	auto res = export_var_defs_.emplace(std::move(name),
		ExportVarDef{
			.export_var_index = uint32_t(export_var_defs_.size()),
			.var_index = var_idx,
		}
	);
	assert(res.second);
}

} // namespace mjs
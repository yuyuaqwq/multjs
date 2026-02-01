#pragma once

#include <string>
#include <vector>

#include <cassert>

namespace mjs {

using VarIndex = uint32_t;
constexpr VarIndex kVarInvaildIndex = 0xffff;

struct VarDef {
	std::string name;
	struct {
		uint32_t is_export : 1;
	} flags;
};

class VarDefTable {
public:
	void AddVar(std::string name) {
		++var_count_;
		var_defs_.emplace_back(std::move(name));
	}

	const auto& GetVarInfo(VarIndex idx) const {
		return var_defs_.at(idx);
	}

	uint32_t var_count() const { return var_count_; }

private:
	uint32_t var_count_ = 0;
	std::vector<VarDef> var_defs_;
};


struct ExportVarDef {
	uint32_t export_var_index;
	VarIndex var_index;
};

class ExportVarDefTable {
public:
	void AddExportVar(std::string name, VarIndex var_idx) {
		auto res = export_var_defs_.emplace(std::move(name),
			ExportVarDef{
				.export_var_index = uint32_t(export_var_defs_.size()),
				.var_index = var_idx,
			}
		);
		assert(res.second);
	}

	const auto& export_var_defs() const { return export_var_defs_; }

private:
	std::unordered_map<std::string, ExportVarDef> export_var_defs_;
};

} // namespace mjs 
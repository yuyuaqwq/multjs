#pragma once

#include <string>
#include <vector>

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

} // namespace mjs 
#include "scope.h"

#include <mjs/error.h>

namespace mjs {
namespace compiler {

const VarInfo& Scope::AllocateVar(const std::string& var_name, VarFlags flags) {
	if (var_table_.find(var_name) != var_table_.end()) {
		throw SyntaxError("local var redefinition: {}.", var_name);
	}
	auto var_idx = function_def_->var_def_table().var_count();
	function_def_->var_def_table().AddVar(var_name);
	auto res = var_table_.emplace(var_name, VarInfo{ .var_idx = var_idx, .flags = flags });
	return res.first->second;
}

const VarInfo* Scope::FindVar(const std::string& var_name) const {
	auto it = var_table_.find(var_name);
	if (it == var_table_.end()) {
		return nullptr;
	}
	return &it->second;
}

} // namespace compiler
} // namespace mjs
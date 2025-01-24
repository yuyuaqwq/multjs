#pragma once

#include <string>
#include <optional>

#include <mjs/func_obj.h>

namespace mjs {

class ScopeException : public std::exception{
public:
	using Base = std::exception;
	using Base::Base;
};

class Scope {
public:
	Scope(FunctionBodyObject* func, uint32_t var_count = 0)
		: func_(func) {}

	uint32_t AllocVar(const std::string& var_name) {
		if (var_table_.find(var_name) != var_table_.end()) {
			throw ScopeException("local var redefinition");
		}
		auto var_idx = func_->var_count++;
		var_table_.emplace(var_name, var_idx);
		return var_idx;
	}

	std::optional<uint32_t> FindVar(const std::string& var_name) {
		auto it = var_table_.find(var_name);
		if (it == var_table_.end()) {
			return std::nullopt;
		}
		return it->second.var_idx;
	}

	FunctionBodyObject* func() const { return func_; }

private:
	FunctionBodyObject* func_; // 所属函数
	struct VarInfo {
		uint32_t var_idx;
		//bool is_upvalue;
	};
	std::unordered_map<std::string, VarInfo> var_table_; // 变量表
};

} // namespace mjs

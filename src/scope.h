#pragma once

#include <string>
#include "func_obj.h"

namespace mjs {

class ScopeException : public std::exception{
public:
	using Base = std::exception;
	using Base::Base;
};
class Scope {
public:
	Scope(FunctionBodyObject* func, uint32_t var_count = 0)
		: func_(func)
		, var_count_(var_count) {}

	uint32_t AllocVar(const std::string& var_name) {
		if (var_table_.find(var_name) != var_table_.end()) {
			throw ScopeException("local var redefinition");
		}
		auto var_idx = var_count_++;
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
	uint32_t var_count() const { return var_count_; }

private:
	FunctionBodyObject* func_; // 所属函数
	uint32_t var_count_; // 当前函数在当前作用域中的有效变量计数
	struct VarInfo {
		uint32_t var_idx;
		bool is_upvalue;
	};
	std::unordered_map<std::string, VarInfo> var_table_; // 变量表
};

} // namespace mjs

#pragma once

#include <string>
#include <optional>

#include <mjs/noncopyable.h>
#include <mjs/function_object.h>

namespace mjs {

class ScopeException : public std::exception{
public:
	using Base = std::exception;
	using Base::Base;
};

class Scope : public noncopyable {
public:
	Scope(FunctionDef* function_def, bool has_finally)
		: function_def_(function_def)
		, has_finally_(has_finally) {}

	Scope(Scope&& other) noexcept {
		operator=(std::move(other));
	}
	void operator=(Scope&& other) noexcept {
		function_def_ = other.function_def_;
		var_table_ = std::move(other.var_table_);
		has_finally_ = other.has_finally_;
	}

	VarIndex AllocVar(const std::string& var_name) {
		if (var_table_.find(var_name) != var_table_.end()) {
			throw ScopeException("local var redefinition");
		}
		auto var_idx = function_def_->var_count();
		function_def_->AddVar(var_name);
		var_table_.emplace(var_name, var_idx);
		return var_idx;
	}

	std::optional<VarIndex> FindVar(const std::string& var_name) {
		auto it = var_table_.find(var_name);
		if (it == var_table_.end()) {
			return std::nullopt;
		}
		return it->second.var_idx;
	}

	FunctionDef* function_def() const { return function_def_; }

	bool has_finally() const { return has_finally_; }

private:
	FunctionDef* function_def_; // 所属函数
	struct VarInfo {
		VarIndex var_idx;
	};
	std::unordered_map<std::string, VarInfo> var_table_; // 变量表

	bool has_finally_;
};

} // namespace mjs

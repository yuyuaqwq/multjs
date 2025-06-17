#pragma once

#include <string>
#include <optional>

#include <mjs/noncopyable.h>
#include <mjs/function_def.h>

namespace mjs {
namespace compiler {

enum class ScopeType {
	kNone = 0,
	kIf,
	kElseIf,
	kElse,
	kWhile,
	kFor,
	kFunction,
	kArrowFunction,

	kTry,
	kTryFinally,
	kCatch,
	kCatchFinally,
	kFinally,
};

enum class VarFlags {
	kNone = 0,
	kConst = 1 << 0,
};

constexpr VarFlags operator|(VarFlags lhs, VarFlags rhs) {
	using underlying = std::underlying_type_t<VarFlags>;
	return static_cast<VarFlags>(
		static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
		);
}

constexpr VarFlags operator&(VarFlags lhs, VarFlags rhs) {
	using underlying = std::underlying_type_t<VarFlags>;
	return static_cast<VarFlags>(
		static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
		);
}

constexpr VarFlags& operator|=(VarFlags& lhs, VarFlags rhs) {
	lhs = lhs | rhs;
	return lhs;
}

struct VarInfo {
	VarIndex var_idx;
	VarFlags flags;
};

class Scope : public noncopyable {
public:
	Scope(FunctionDef* function_def, ScopeType type)
		: function_def_(function_def)
		, type_(type) {}

	Scope(Scope&& other) noexcept {
		operator=(std::move(other));
	}
	void operator=(Scope&& other) noexcept {
		function_def_ = other.function_def_;
		var_table_ = std::move(other.var_table_);
		type_ = other.type_;
	}

	const VarInfo& AllocVar(const std::string& var_name, VarFlags flags) {
		if (var_table_.find(var_name) != var_table_.end()) {
			throw std::runtime_error("local var redefinition");
		}
		auto var_idx = function_def_->var_def_table().var_count();
		function_def_->var_def_table().AddVar(var_name);
		auto res = var_table_.emplace(var_name, VarInfo{ .var_idx = var_idx, .flags = flags });
		return res.first->second;
	}

	const VarInfo* FindVar(const std::string& var_name) const {
		auto it = var_table_.find(var_name);
		if (it == var_table_.end()) {
			return nullptr;
		}
		return &it->second;
	}

	FunctionDef* function_def() const { return function_def_; }

	ScopeType type() const { return type_; }

private:
	FunctionDef* function_def_; // 所属函数
	std::unordered_map<std::string, VarInfo> var_table_; // 变量表

	ScopeType type_;
};

} // namespace compiler
} // namespace mjs

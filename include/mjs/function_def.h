#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/reference_counter.h>
#include <mjs/exception.h>
#include <mjs/var_def.h>
#include <mjs/bytecode.h>

namespace mjs {

struct ClosureVarDef {
	// 该变量在ClosureEnvironment::vars_的索引
	uint32_t env_var_idx;

	// 在父作用域中的变量索引
	VarIndex parent_var_idx;
};

class Runtime;
// 不会有循环引用问题，仅使用引用计数管理
class FunctionDef : public ReferenceCounter {
public:
	struct VarDef {
		std::string name;
		struct {
			uint32_t is_export : 1;
		} flags;
	};

public:
	FunctionDef(Runtime* runtime, std::string name, uint32_t par_count) noexcept;

	std::string Disassembly(Context* context) const;

	void SetGenerator() {
		type_ = FunctionType::kGenerator;
	}

	void SetAsync() {
		type_ = FunctionType::kAsync;
	}

	void SetModule() {
		type_ = FunctionType::kModule;
	}

	bool IsNormal() const {
		return type_ == FunctionType::kNormal;
	}

	bool IsGenerator() const {
		return type_ == FunctionType::kGenerator;
	}

	bool IsAsync() const {
		return type_ == FunctionType::kAsync;
	}

	bool IsModule() const {
		return type_ == FunctionType::kModule;
	}

	FunctionType type() {
		return type_;
	}

	void AddVar(std::string name) {
		++var_count_;
		var_defs_.emplace_back(std::move(name));
	}

	const auto& GetVarInfo(VarIndex idx) const {
		return var_defs_.at(idx);
	}

	void AddClosureVar(VarIndex var_idx, VarIndex parent_var_idx) {
		closure_var_defs_.emplace(var_idx, 
			ClosureVarDef{
				.env_var_idx = uint32_t(closure_var_defs_.size()),
				.parent_var_idx = parent_var_idx,
			}
		);
	}

	const auto& name() const { return name_; }

	auto par_count() const { return par_count_; }
	auto var_count() const { return var_count_; }

	const auto& byte_code() const { return byte_code_; }
	auto& byte_code() { return byte_code_; }

	const auto& closure_var_defs() const { return closure_var_defs_; }
	auto& closure_var_defs() { return closure_var_defs_; }

	const auto& exception_table() const { return exception_table_; }
	auto& exception_table() { return exception_table_; }

protected:
	Runtime* runtime_;

	std::string name_;
	FunctionType type_ = FunctionType::kNormal;

	// 字节码
	ByteCode byte_code_;

	// 变量
	uint32_t par_count_;
	uint32_t var_count_ = 0;
	std::vector<VarDef> var_defs_;
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;		// 捕获的外部变量

	// 异常
	ExceptionTable exception_table_;
};

} // namespace mjs


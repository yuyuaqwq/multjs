#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/reference_counter.h>
#include <mjs/exception.h>
#include <mjs/var_def.h>
#include <mjs/bytecode.h>
#include <mjs/debug.h>

namespace mjs {

struct ClosureVarDef {
	// 该变量在ClosureEnvironment::vars_的索引
	uint32_t env_var_idx;

	// 在父作用域中的变量索引
	VarIndex parent_var_idx;
};

class Runtime;
// 不会有循环引用问题，仅使用引用计数管理
class FunctionDef : public ReferenceCounter<FunctionDef> {
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

	void set_is_normal() {
		flags_.is_normal_ = true;
		flags_.is_module_ = false;
		flags_.is_arrow_ = false;
	}

	void set_is_module() {
		flags_.is_normal_ = false;
		flags_.is_module_ = true;
		flags_.is_arrow_ = false;
	}

	void set_is_arrow() {
		flags_.is_normal_ = false;
		flags_.is_module_ = false;
		flags_.is_arrow_ = true;
	}

	void set_is_generator() {
		flags_.is_generator_ = true;
	}

	void set_is_async() {
		flags_.is_asnyc_ = true;
	}

	bool is_normal() const {
		return flags_.is_normal_;
	}

	bool is_module() const {
		return flags_.is_module_;
	}

	bool is_arrow() const {
		return flags_.is_arrow_;
	}

	bool is_generator() const {
		return flags_.is_generator_;
	}

	bool is_async() const {
		return flags_.is_asnyc_;
	}


	auto par_count() const { return par_count_; }
	auto var_count() const { return var_count_; }

	const auto& byte_code() const { return byte_code_; }
	auto& byte_code() { return byte_code_; }

	const auto& closure_var_defs() const { return closure_var_defs_; }
	auto& closure_var_defs() { return closure_var_defs_; }

	const auto& has_this() const { return has_this_; }
	void set_has_this(bool has_this) { has_this_ = has_this; }
	
	const auto& exception_table() const { return exception_table_; }
	auto& exception_table() { return exception_table_; }

	const auto& debug_table() const { return debug_table_; }
    auto& debug_table() { return debug_table_; }

protected:
	Runtime* runtime_;

	std::string name_;
	// FunctionType type_ = FunctionType::kNormal;

	struct {
		uint32_t is_normal_ : 1 = 0;
		uint32_t is_module_ : 1 = 0;
		uint32_t is_arrow_ : 1 = 0;
		uint32_t is_generator_ : 1 = 0;
		uint32_t is_asnyc_ : 1 = 0;
	} flags_;

	// 字节码
	BytecodeTable byte_code_;

	// 变量
	uint32_t par_count_;
	uint32_t var_count_ = 0;
	std::vector<VarDef> var_defs_;
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;		// 捕获的外部变量
	bool has_this_ = false;

	// 异常
	ExceptionTable exception_table_;

	// 调试
	DebugTable debug_table_;
};

} // namespace mjs


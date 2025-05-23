#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/reference_counter.h>
#include <mjs/exception.h>
#include <mjs/variable.h>
#include <mjs/bytecode.h>
#include <mjs/closure.h>
#include <mjs/debug.h>

namespace mjs {

class ModuleDef;
// 不会有循环引用问题，仅使用引用计数管理
class FunctionDef : public ReferenceCounter<FunctionDef> {
public:
	FunctionDef(ModuleDef* module_def, std::string name, uint32_t par_count) noexcept;

	std::string Disassembly(Context* context) const;

	const auto& module_def() const { return *module_def_; }
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
	


	const auto& bytecode_table() const { return bytecode_table_; }
	auto& bytecode_table() { return bytecode_table_; }

	const auto& var_def_table() const { return var_def_table_; }
	auto& var_def_table() { return var_def_table_; }

	const auto& closure_var_table() const { return closure_var_table_; }
	auto& closure_var_table() { return closure_var_table_; }

	const auto& has_this() const { return has_this_; }
	void set_has_this(bool has_this) { has_this_ = has_this; }
	
	const auto& exception_table() const { return exception_table_; }
	auto& exception_table() { return exception_table_; }

	const auto& debug_table() const { return debug_table_; }
    auto& debug_table() { return debug_table_; }

protected:
	ModuleDef* module_def_;

	std::string name_;
	// FunctionType type_ = FunctionType::kNormal;

	struct {
		uint32_t is_normal_ : 1 = 0;
		uint32_t is_module_ : 1 = 0;
		uint32_t is_arrow_ : 1 = 0;
		uint32_t is_generator_ : 1 = 0;
		uint32_t is_asnyc_ : 1 = 0;
	} flags_;

	uint32_t par_count_;

	// 字节码
	BytecodeTable bytecode_table_;

	// 变量
	VarDefTable var_def_table_;

	// 闭包
	ClosureVarTable closure_var_table_;

	bool has_this_ = false;

	// 异常
	ExceptionTable exception_table_;

	// 调试
	DebugTable debug_table_;
};

} // namespace mjs


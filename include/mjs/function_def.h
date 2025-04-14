#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/object.h>
#include <mjs/exception.h>
#include <mjs/bytecode.h>

namespace mjs {


struct ClosureVarDef {
	// upvalue在closure_value_arr_的索引
	uint32_t arr_idx;

	// upvalue指向的变量在父作用域中的变量索引
	std::optional<VarIndex> parent_var_idx;
};

class FunctionDef : public noncopyable {
public:
	struct VarInfo {
		std::string name;
	};

public:
	explicit FunctionDef(uint32_t par_count) noexcept;

	std::string Disassembly(Context* context);

	void SetGenerator() {
		type_ = FunctionType::kGenerator;
	}

	void SetAsync() {
		type_ = FunctionType::kAsync;
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

	FunctionType type() {
		return type_;
	}

	void AddVar(std::string name) {
		++var_count_;
		var_info_.emplace_back(std::move(name));
	}

	const auto& GetVarInfo(VarIndex idx) const {
		return var_info_.at(idx);
	}

	auto par_count() const { return par_count_; }
	auto var_count() const { return var_count_; }

	const auto& byte_code() const { return byte_code_; }
	auto& byte_code() { return byte_code_; }

	const auto& closure_var_defs() const { return closure_var_defs_; }
	auto& closure_var_defs() { return closure_var_defs_; }

	const auto& exception_table() const { return exception_table_; }
	auto& exception_table() { return exception_table_; }

private:
	uint32_t par_count_;
	uint32_t var_count_ = 0;		// 包括par_count

	ByteCode byte_code_;

	FunctionType type_ = FunctionType::kNormal;

	// 优化方向：
	// 如果所有记录都没有捕获外部变量，都是顶级upvalue变量
	// 则closure_value_arr_可以指向栈帧，无需内存分配

	// upvalue变量记录，upvalue变量在当前作用域的索引
	// 如果存在的话，则加载时需要创建FunctionObject

	// key: upvalue变量索引
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;

	std::vector<VarInfo> var_info_;

	// 异常
	ExceptionTable exception_table_;
};

} // namespace mjs


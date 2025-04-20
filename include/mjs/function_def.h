#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/exception.h>
#include <mjs/var_def.h>
#include <mjs/bytecode.h>
#include <mjs/value.h>

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
		struct {
			uint32_t is_export : 1;
		} flags;
	};

public:
	FunctionDef(std::string name, uint32_t par_count) noexcept;

	std::string Disassembly(Context* context);

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
		var_info_.emplace_back(std::move(name));
	}

	const auto& GetVarInfo(VarIndex idx) const {
		return var_info_.at(idx);
	}

	void AddClosureVar(VarIndex var_idx, std::optional<VarIndex> parent_var_idx) {
		closure_var_defs_.emplace(var_idx, 
			ClosureVarDef{
				.arr_idx = uint32_t(closure_var_defs_.size()),
				.parent_var_idx = parent_var_idx,
			}
		);
	}

	void AddExport(std::string_view name, VarIndex var_idx) {
		exports_.emplace(Value(name.data()), var_idx);
	}

	const auto& name() const { return name_; }

	auto par_count() const { return par_count_; }
	auto var_count() const { return var_count_; }

	const auto& byte_code() const { return byte_code_; }
	auto& byte_code() { return byte_code_; }

	const auto& exports() const { return exports_; }
	auto& exports() { return exports_; }

	const auto& closure_var_defs() const { return closure_var_defs_; }
	auto& closure_var_defs() { return closure_var_defs_; }

	const auto& exception_table() const { return exception_table_; }
	auto& exception_table() { return exception_table_; }

private:
	std::string name_;

	uint32_t par_count_;
	uint32_t var_count_ = 0;		// 包括par_count

	std::vector<VarInfo> var_info_;

	std::unordered_map<Value, VarIndex, ValueHash> exports_;

	ByteCode byte_code_;

	FunctionType type_ = FunctionType::kNormal;

	// 优化方向：
	// 如果所有记录都没有捕获外部变量，都是顶级upvalue变量
	// 则closure_value_arr_可以指向栈帧，无需内存分配

	// upvalue变量记录，upvalue变量在当前作用域的索引
	// 如果存在的话，则加载时需要创建FunctionObject

	// key: upvalue变量索引
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;

	// 异常
	ExceptionTable exception_table_;
};

} // namespace mjs


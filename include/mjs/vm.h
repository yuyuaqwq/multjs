#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <mjs/value.h>
#include <mjs/const_pool.h>
#include <mjs/var_def.h>
#include <mjs/stack_frame.h>
#include <mjs/value.h>

namespace mjs {

class VmException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Context;
class Vm : public noncopyable {
public:
	friend class CodeGener;

public:
	explicit Vm(Context* context);

	Value CallFunction(Value func, Value this_val, const std::vector<Value>& argv);

	Value& GetVar(VarIndex idx);
	void SetVar(VarIndex idx, Value&& var);

private:
	bool FunctionDefLoadInit(Value* func_def_val);
	void FunctionEnterInit(const Value& func_val);

	// 返回是否需要继续执行字节码
	bool FunctionSwitch(Value func_val, Value this_val);

	void CallInternal(Value func_val, Value this_val);

	const Value& GetGlobalConst(ConstIndex idx);
	const Value& GetLocalConst(ConstIndex idx);
	const Value& GetConst(ConstIndex idx);

	void LoadConst(ConstIndex const_idx);

	void SaveStackFrame(const Value& func_val, FunctionDef* func_def
		, Value&& this_val, uint32_t par_count, bool is_generator);
	Value RestoreStackFrame();

	Stack& stack();
	FunctionDef* function_def(const Value& func_val) const;
private:
	Context* context_;

	Value cur_func_val_;
	FunctionDef* cur_func_def_;
	uint32_t pc_ = 0;

	StackFrame stack_frame_;
};

} // namespace mjs
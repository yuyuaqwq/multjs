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

	Value CallFunction(const StackFrame& upper_stack_frame, Value func, Value this_val, const std::vector<Value>& argv);

	Value& GetVar(StackFrame* stack_frame, VarIndex idx);
	void SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var);

private:
	bool InitClosure(const StackFrame& upper_stack_frame, Value* func_def_val);
	void BindClosureVars(StackFrame* stack_frame, const Value& func_val);

	// 返回是否需要继续执行字节码
	bool FunctionSwitch(StackFrame* stack_frame, Value* func_val, Value this_val, uint32_t par_count);

	void CallInternal(StackFrame* stack_frame, Value func_val, Value this_val);

	const Value& GetGlobalConst(ConstIndex idx);
	const Value& GetLocalConst(ConstIndex idx);
	const Value& GetConst(StackFrame* stack_frame, ConstIndex idx);

	void LoadConst(StackFrame* stack_frame, ConstIndex const_idx);

	void SwitchStackFrame(const Value& func_val, FunctionDef* func_def
		, Value&& this_val, uint32_t par_count, bool is_generator);

	bool ThrowExecption(FunctionDef* cur_func_def, Pc* pc, std::optional<Value>* error_val);

	Stack& stack();
	FunctionDef* function_def(const Value& func_val) const;

private:
	Context* context_;
	// StackFrame stack_frame_;
};

} // namespace mjs
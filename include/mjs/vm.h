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
class Vm {
public:
	friend class CodeGener;

public:
	explicit Vm(Context* context);

public:
	void EvalFunction(const Value& func);

private:
	bool FunctionDefLoadInit(Value* func_def_val);
	void FunctionEnterInit(const Value& func_val);
	void FunctionSwitch(FunctionDef** cur_func_def, const Value& func_val);

	void Run();

	Value& GetVar(VarIndex idx);
	void SetVar(VarIndex idx, Value&& var);

	const Value& GetGlobalConst(ConstIndex idx);
	const Value& GetLocalConst(ConstIndex idx);
	const Value& GetConst(ConstIndex idx);

	void LoadConst(ConstIndex const_idx);

	Value SaveStackFrame();
	Value RestoreStackFrame(FunctionDef** cur_func_def);

	Stack& stack();
	FunctionDef* function_def(const Value& func_val) const;
private:
	Context* context_;

	Value cur_func_val_;
	uint32_t pc_ = 0;

	Value this_val_;

	StackFrame stack_frame_;
};

} // namespace mjs
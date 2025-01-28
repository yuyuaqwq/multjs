#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <mjs/value.h>
#include <mjs/const_pool.h>
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
	bool FunctionDefInit(Value* func_val);
	void FunctionInit(const Value& func_val);
	void FunctionSwitch(FunctionDefObject** cur_func_def, const Value& func_val);

	void Run();

	Value& GetVar(uint32_t idx);
	void SetVar(uint32_t idx, Value&& var);

	void LoadConst(uint32_t const_idx);

	const ConstPool& const_pool() const;
	Stack& stack();
	FunctionDefObject* function_def(const Value& func_val) const;
private:
	Context* context_;

	Value cur_func_val_;
	uint32_t pc_ = 0;

	StackFrame stack_frame_;
};

} // namespace mjs
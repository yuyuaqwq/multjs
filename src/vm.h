#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "instr.h"

#include "value.h"
#include "const_pool.h"
#include "stack_frame.h"
#include "object.h"

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
	void SetEvalFunction(const Value& func);

	void Run();

private:
	Value& GetVar(uint32_t idx);
	void SetVar(uint32_t idx, Value&& var);

	const ConstPool& const_pool() const;

private:
	Context* context_;

	FunctionBodyObject* cur_func_ = nullptr;
	uint32_t pc_ = 0;

	StackFrame stack_frame_;
};

} // namespace mjs
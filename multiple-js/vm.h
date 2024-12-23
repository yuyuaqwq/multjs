#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>

#include "instr.h"

#include "value.h"
#include "const_pool.h"
#include "stack_frame.h"

namespace mjs {

class VMException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class VM {
public:
	friend class CodeGener;

public:
	explicit VM(ConstPool* const_pool);

public:
	std::string Disassembly();
	void Run();

private:
	Value* GetVar(uint32_t idx);
	Value GetVarCopy(uint32_t idx);
	void SetVar(uint32_t idx, Value&& var);
	void SetVar(uint32_t idx, Value* var);

private:
	uint32_t pc_ = 0;
	FunctionBodyObject* cur_func_;
	ConstPool* const_pool_;
	StackFrame stack_sect_;
};

} // namespace mjs
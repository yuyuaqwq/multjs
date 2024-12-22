#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>

#include "instr.h"

#include "value.h"
#include "section.h"

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
	explicit VM(ValueSection* const_sect);

public:
	std::string Disassembly();
	void Run();

private:
	Value* GetVar(uint32_t idx);
	std::unique_ptr<Value> GetVarCopy(uint32_t idx);
	void SetVar(uint32_t idx, std::unique_ptr<Value> var);
	void SetVar(uint32_t idx, Value* var);

private:
	uint32_t pc_ = 0;
	FunctionBodyValue* cur_func_;
	ValueSection* const_sect_;
	ValueSection stack_sect_;
};

} // namespace mjs
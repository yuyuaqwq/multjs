#pragma once

#include "stack_frame.h"

namespace mjs {

class Object {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_ = 1;
	}


private:
	union {
		uint64_t full_;
		uint32_t ref_;
	} tag_;
};

// 关于函数的设计
// 函数体就是指令流的封装，只会存放在常量里，定义函数会在常量池创建函数
// 并且会在局部变量表中创建并函数原型，指向函数体，类似语法糖的想法
class FunctionBodyObject : public Object {
public:
	explicit FunctionBodyObject(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	ByteCode byte_code;
	StackFrame stack_frame;
};

class UpValueObject : public Object {
public:
	UpValueObject(uint32_t t_index, FunctionBodyObject* func_body) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_body;
};

} // namespace mjs
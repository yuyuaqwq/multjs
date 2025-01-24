#pragma once

#include <string>

#include <mjs/object.h>
#include <mjs/value.h>

#include "instr.h"

namespace mjs {

// 关于函数的设计
// 函数体就是指令流的封装，只会存放在常量里，定义函数会在常量池创建函数
// 并且会在局部变量表中创建并函数引用，指向函数体，类似语法糖的想法
class FunctionBodyObject : public Object {
public:
	explicit FunctionBodyObject(uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	// 引用的闭包变量

};

class FunctionRefObject : public Object {
public:
	explicit FunctionRefObject(FunctionBodyObject* func_body) noexcept;

public:
	FunctionBodyObject* func_body_;

	Value parent_func_;
	// 被捕获的变量列表

};

} // namespace mjs
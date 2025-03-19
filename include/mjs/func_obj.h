#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/object.h>
#include <mjs/value.h>

#include "instr.h"

namespace mjs {

class FunctionDefObject : public Object {
public:
	explicit FunctionDefObject(uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	// 优化方向：
	// 如果所有记录都没有捕获外部变量，都是顶级upvalue变量
	// 则closure_value_arr_可以指向栈帧，无需内存分配

	// upvalue变量记录，upvalue变量在当前作用域的索引
	// 如果存在的话，则加载时需要创建FunctionObject
	struct ClosureVarDef {
		// upvalue在closure_value_arr_的索引
		int32_t arr_idx;

		// upvalue指向的变量在父作用域中的变量索引
		std::optional<VarIndex> parent_var_idx;
	};
	// key: upvalue变量索引
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;
};

// 闭包，可以考虑改名ClosureObject
// 但目前不一定是闭包，只是变量被子函数捕获的函数也生成这个函数
// 闭包就是在将FuncDefObject赋值给Value的时候，会捕获当前的词法作用域上下文，闭包对象=函数指针+外部变量的捕获列表
class FunctionObject : public Object {
public:
	explicit FunctionObject(FunctionDefObject* def) noexcept;

public:
	FunctionDefObject* func_def_;

	// 父函数的引用计数占用
	// 用于当前闭包被返回时，延长父函数的ArrayValue的生命周期
	Value parent_function_;

	// 当前函数被子函数捕获的值，不在放到栈上，而是提升到堆上(包括UpValue)
	std::vector<Value> closure_value_arr_;
};

} // namespace mjs
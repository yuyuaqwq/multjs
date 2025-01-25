#pragma once

#include <string>
#include <unordered_map>

#include <mjs/object.h>
#include <mjs/value.h>

#include "instr.h"

namespace mjs {

class FunctionDefObject : public Object {
public:
	explicit FunctionDefObject(FunctionDefObject* parent, uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	FunctionDefObject* parent;

	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	// upvalue变量记录，upvalue变量在当前作用域的索引
	// 如果存在的话，则加载时需要创建FunctionObject
	struct ClosureVarDef {
		int32_t arr_idx;
		int32_t parent_var_idx;
	};
	// key: var_idx
	std::unordered_map<int32_t, ClosureVarDef> closure_var_defs_;
};

// 闭包，可以考虑改名ClosureObject
// 但目前不一定是闭包，只是变量被子函数捕获的函数也生成这个函数
class FunctionObject : public Object {
public:
	explicit FunctionObject(FunctionDefObject* def) noexcept;

public:
	FunctionDefObject* func_def_;

	// 父函数的引用计数占用
	// 用于当前闭包被返回时，延长生命周期
	Value parent_closure_value_arr_;

	// 当前函数被子函数捕获的值，不在放到栈上，而是提升到堆上(包括UpValue)
	Value closure_value_arr_;
};

} // namespace mjs
#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/object.h>
#include <mjs/function_def.h>

namespace mjs {

// 闭包，可以考虑改名ClosureObject
// 但目前不一定是闭包，只是变量被子函数捕获的函数也生成这个函数
// 闭包就是在将FuncDef赋值给Value的时候，会捕获当前的词法作用域上下文，闭包对象=函数指针+外部变量的捕获列表
class FunctionObject : public Object {
public:
	explicit FunctionObject(FunctionDef* def) noexcept;

public:
	FunctionDef* func_def_;

	// 父函数的引用计数占用
	// 用于当前闭包被返回时，延长父函数的ArrayValue的生命周期
	Value parent_function_;

	// 当前函数被子函数捕获的值，不在放到栈上，而是提升到堆上(包括UpValue)
	std::vector<Value> closure_value_arr_;
};

} // namespace mjs
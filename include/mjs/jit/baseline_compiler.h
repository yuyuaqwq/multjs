/**
 * @file baseline_compiler.h
 * @brief Baseline JIT编译器
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了Baseline JIT编译器，将字节码编译为机器码
 */

#pragma once

#include <mjs/opcode.h>
#include <mjs/variable.h>

#ifdef ENABLE_JIT

#include <unordered_map>

// 前向声明
namespace mjs {
class Context;
class FunctionDefBase;
class StackFrame;
class Value;

namespace jit {

class BaselineCompiler {
public:
	explicit BaselineCompiler(Context* context);
	~BaselineCompiler();

	BaselineCompiler(const BaselineCompiler&) = delete;
	BaselineCompiler& operator=(const BaselineCompiler&) = delete;
	BaselineCompiler(BaselineCompiler&&) = delete;
	BaselineCompiler& operator=(BaselineCompiler&&) = delete;

	void* Compile(FunctionDefBase* func_def);

private:
	class Impl;
	Impl* impl_;

	bool CompileFunction(FunctionDefBase* func_def);
	void EmitReturn();
};

} // namespace jit
} // namespace mjs

#endif // ENABLE_JIT

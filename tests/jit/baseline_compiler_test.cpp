/**
 * @file baseline_compiler_test.cpp
 * @brief BaselineCompiler集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <mjs/jit/baseline_compiler.h>
#include <mjs/jit/jit_manager.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <test_helpers.h>

using namespace mjs;
using namespace mjs::jit;
using namespace mjs::test;

/**
 * @test 测试BaselineCompiler基础编译功能
 */
TEST(BaselineCompiler, BasicCompilation) {
#ifdef ENABLE_JIT
	TestEnvironment test_env;

	// 创建Context
	Context context(test_env.runtime());

	// 创建函数定义
	auto* func_def = test_env.CreateFunctionDef("testFunc", 0);

	JITManager jit_manager(&context);

	// 添加基础字节码指令
	// 注意：当前 BaselineCompiler 只实现了 Return 指令的处理
	// 所以这里只生成 Return 指令来测试基础编译流程
	auto& bytecode_table = func_def->bytecode_table();
	bytecode_table.EmitReturn(func_def);

	// 编译函数 (注意：当前 BaselineCompiler 只实现了 Return 指令)
	// 因此我们只生成 Return 指令来测试基础编译流程
	BaselineCompiler compiler(&context);
	void* code_ptr = compiler.Compile(func_def);

	// 验证编译成功
	// 注意：由于 BaselineCompiler 尚未完整实现所有指令，
	// 这里测试可能会失败，这是预期的行为
	EXPECT_NE(code_ptr, nullptr);

	// 清理编译的代码
	if (code_ptr) {
		// JIT代码会在函数定义析构时自动释放
	}
#endif
}

/**
 * @test 测试JITManager基础功能
 */
TEST(JITManager, BasicFunctionality) {
#ifdef ENABLE_JIT
	TestEnvironment test_env;
	Context context(test_env.runtime());

	// 创建JITManager
	JITManager jit_manager(&context);

	// 创建函数定义
	auto* func_def = test_env.CreateFunctionDef("testFunc", 0);
	auto& bytecode_table = func_def->bytecode_table();

	// 添加简单的字节码 (只包含 Return)
	bytecode_table.EmitReturn(func_def);

	// 编译函数
	jit_manager.CompileBaseline(func_def);

	// 验证代码已编译
	auto* jit_code = jit_manager.GetBaselineCode(func_def);
	EXPECT_NE(jit_code, nullptr);
#endif
}

/**
 * @test 测试热点计数器功能
 */
TEST(JITManager, HotnessCounter) {
#ifdef ENABLE_JIT
	TestEnvironment test_env;
	Context context(test_env.runtime());

	JITManager jit_manager(&context);

	auto* func_def = test_env.CreateFunctionDef("testFunc", 0);

	auto& hotness_counter = func_def->hotness_counter();

	// 初始状态
	EXPECT_EQ(hotness_counter.state(), ExecutionState::kInterpreted);
	EXPECT_EQ(hotness_counter.count(), 0);

	// 执行99次，不应触发Baseline
	for (int i = 0; i < 99; i++) {
		hotness_counter.Increment();
		EXPECT_EQ(hotness_counter.state(), ExecutionState::kInterpreted);
	}

	// 第100次触发
	hotness_counter.Increment();
	EXPECT_EQ(hotness_counter.state(), ExecutionState::kWarmup);
	EXPECT_EQ(hotness_counter.count(), 100);
#endif
}

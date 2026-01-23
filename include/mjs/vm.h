/**
 * @file vm.h
 * @brief JavaScript虚拟机核心实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了JavaScript虚拟机的核心执行引擎，负责字节码解释执行、
 * 函数调用、闭包处理等关键功能。
 */

#pragma once

#include <stdint.h>

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include <mjs/value.h>
#include <mjs/variable.h>
#include <mjs/stack_frame.h>

namespace mjs {

/**
 * @brief 测试命名空间，包含解析器测试相关的类
 */
namespace test {
/**
 * @class VMTest
 * @brief 虚拟机测试类，用于单元测试
 */
class VMTest;
class VMModuleTest;
class VMClosureTest;
class VMFunctionSchedulingTest;
class VMBytecodeExecutionTest;
class VMExceptionTest;
class VMGeneratorTest;
class VMIntegrationTest;
} // namespace test

class Context;

/**
 * @class VM
 * @brief JavaScript虚拟机核心类
 *
 * 负责执行JavaScript字节码，管理函数调用栈，处理闭包和异常等。
 * 继承自noncopyable确保单例特性。
 */
class VM : public noncopyable {
public:
	friend class CodeGenerator;
	friend class ::mjs::test::VMTest;
	friend class ::mjs::test::VMModuleTest;
	friend class ::mjs::test::VMClosureTest;
	friend class ::mjs::test::VMFunctionSchedulingTest;
	friend class ::mjs::test::VMBytecodeExecutionTest;
	friend class ::mjs::test::VMExceptionTest;
	friend class ::mjs::test::VMGeneratorTest;
	friend class ::mjs::test::VMIntegrationTest;

public:
	/**
	 * @brief 构造函数
	 * @param context 执行上下文指针
	 */
	explicit VM(Context* context);

	/**
	 * @brief 模块初始化
	 * @param value 模块值指针
	 */
	void ModuleInit(Value* value);

	/**
	 * @brief 绑定模块导出变量
	 * @param stack_frame 栈帧指针
	 */
	void BindModuleExportVars(StackFrame* stack_frame);

	/**
	 * @brief 调用JavaScript函数
	 * @tparam It 迭代器类型
	 * @param stack_frame 栈帧指针
	 * @param func_val 函数值
	 * @param this_val this值
	 * @param begin 参数起始迭代器
	 * @param end 参数结束迭代器
	 * @return 函数返回值
	 */
	template<typename It>
	Value CallFunction(StackFrame* stack_frame, Value func_val, Value this_val, It begin, It end) {
		// 参数正序入栈
		for (It it = begin; it != end; ++it) {
			stack_frame->push(*it);
		}
		CallInternal(stack_frame, std::move(func_val), std::move(this_val), std::distance(begin, end));
		return stack_frame->pop();
}

private:
	/**
	 * @brief 获取变量值
	 * @param stack_frame 栈帧指针
	 * @param idx 变量索引
	 * @return 变量值的引用
	 */
	Value& GetVar(const StackFrame& stack_frame, VarIndex idx);

	/**
	 * @brief 设置变量值
	 * @param stack_frame 栈帧指针
	 * @param idx 变量索引
	 * @param var 变量值
	 */
	void SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var);
	/**
	 * @brief 创建闭包
	 * @param stack_frame 栈帧指针
	 * @param func_def_val 函数定义值
	 */
	void Closure(const StackFrame& stack_frame, Value* func_def_val);

	/**
	 * @brief 绑定闭包变量
	 * @param stack_frame 栈帧指针
	 */
	void BindClosureVars(StackFrame* stack_frame);

	/**
	 * @brief 函数调度
	 * @param stack_frame 栈帧指针
	 * @param par_count 参数数量
	 * @return 是否需要继续执行字节码
	 */
	bool FunctionScheduling(StackFrame* stack_frame, uint32_t par_count);

	/**
	 * @brief 内部函数调用实现
	 * @param stack_frame 栈帧指针
	 * @param func_val 函数值
	 * @param this_val this值
	 * @param param_count 参数数量
	 */
	void CallInternal(StackFrame* stack_frame, Value func_val, Value this_val, uint32_t param_count);

	/**
	 * @brief 加载常量
	 * @param stack_frame 栈帧指针
	 * @param const_idx 常量索引
	 */
	void LoadConst(StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief 抛出异常
	 * @param stack_frame 栈帧指针
	 * @param error_val 错误值指针
	 * @return 是否成功抛出异常
	 */
	bool ThrowException(StackFrame* stack_frame, std::optional<Value>* error_val, bool need_inc_pc = false);

private:
	/**
	 * @brief 保存生成器上下文
	 * @param stack_frame 栈帧指针
	 * @param generator 生成器对象
	 */
	void GeneratorSaveContext(StackFrame* stack_frame, GeneratorObject* generator);

	/**
	 * @brief 恢复生成器上下文
	 * @param stack_frame 栈帧指针
	 * @param generator 生成器对象
	 */
	void GeneratorRestoreContext(StackFrame* stack_frame, GeneratorObject* generator);

	/**
	 * @brief 获取栈引用
	 * @return 栈引用
	 */
	Stack& stack();

private:
	Context* context_; ///< 执行上下文指针
	// StackFrame stack_frame_; ///< 栈帧（已注释）
};

} // namespace mjs
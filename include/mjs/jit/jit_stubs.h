/**
 * @file jit_stubs.h
 * @brief JIT辅助函数和存根
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了JIT编译器调用的辅助函数，用于回退到解释器执行
 */

#pragma once

#include <mjs/opcode.h>
#include <mjs/variable.h>

#ifdef ENABLE_JIT

namespace mjs {

class Context;
class StackFrame;
class Value;

namespace jit {

/**
 * @brief JIT辅助函数集合
 *
 * 这些函数由JIT编译的代码调用，用于执行复杂操作或回退到解释器
 */
struct JitStubs {
	/**
	 * @brief 从常量池加载常量
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param const_idx 常量索引
	 */
	static void LoadConst(Context* context, StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief 加载变量
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param var_idx 变量索引
	 */
	static void LoadVar(Context* context, StackFrame* stack_frame, VarIndex var_idx);

	/**
	 * @brief 存储变量
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param var_idx 变量索引
	 */
	static void StoreVar(Context* context, StackFrame* stack_frame, VarIndex var_idx);

	/**
	 * @brief 弹出栈顶值
	 * @param stack_frame 栈帧
	 */
	static void Pop(StackFrame* stack_frame);

	/**
	 * @brief 加载全局变量
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param const_idx 属性名常量索引
	 */
	static void LoadGlobal(Context* context, StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief 加载属性
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param const_idx 属性名常量索引
	 */
	static void LoadProperty(Context* context, StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief 存储属性
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param const_idx 属性名常量索引
	 */
	static void StoreProperty(Context* context, StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief 二元加法
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Add(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 二元减法
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Sub(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 二元乘法
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Mul(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 二元除法
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Div(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 二元取模
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Mod(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 一元取反
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Neg(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 前缀递增
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Inc(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 相等比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Eq(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 不等比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Ne(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 小于比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Lt(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 小于等于比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Le(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 大于比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Gt(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 大于等于比较
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Ge(Context* context, StackFrame* stack_frame);

	/**
	 * @brief typeof操作
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Typeof(Context* context, StackFrame* stack_frame);

	/**
	 * @brief toString操作
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void ToString(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 函数调用
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void FunctionCall(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 获取this值
	 * @param stack_frame 栈帧
	 */
	static void GetThis(StackFrame* stack_frame);

	/**
	 * @brief 获取外层this值
	 * @param stack_frame 栈帧
	 */
	static void GetOuterThis(StackFrame* stack_frame);

	/**
	 * @brief 创建闭包
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 * @param const_idx 函数定义常量索引
	 */
	static void Closure(Context* context, StackFrame* stack_frame, ConstIndex const_idx);

	/**
	 * @brief new操作
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void New(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 索引加载
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void IndexedLoad(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 索引存储
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void IndexedStore(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 左移运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Shl(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 右移运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void Shr(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 无符号右移运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void UShr(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 按位与运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void BitAnd(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 按位或运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void BitOr(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 按位异或运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void BitXor(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 按位取反运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void BitNot(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 逻辑与运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void LogicalAnd(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 逻辑或运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void LogicalOr(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 空值合并运算
	 * @param context 执行上下文
	 * @param stack_frame 栈帧
	 */
	static void NullishCoalescing(Context* context, StackFrame* stack_frame);

	/**
	 * @brief 判断值是否为falsy
	 * @param value 指向要判断的值的指针
	 * @return true表示falsy（undefined, null, false, 0, NaN, ""），false表示truthy
	 */
	static bool IsFalsy(const Value* value);

	/**
	 * @brief 从栈帧获取值
	 * @param stack_frame 栈帧指针
	 * @param offset 偏移量
	 * @return 值的指针
	 */
	static Value* Get(StackFrame* stack_frame, int64_t offset);

	/**
	 * @brief 向栈帧压入值
	 * @param stack_frame 栈帧指针
	 * @param value_ptr 值指针
	 */
	static void PushValue(StackFrame* stack_frame, Value* value_ptr);

	/**
	 * @brief 交换栈顶两个值
	 * @param stack_frame 栈帧指针
	 */
	static void SwapStub(StackFrame* stack_frame);
};

} // namespace jit
} // namespace mjs

#endif // ENABLE_JIT

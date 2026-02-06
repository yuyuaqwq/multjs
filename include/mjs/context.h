/**
 * @file context.h
 * @brief JavaScript 执行上下文管理
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 执行上下文，负责管理模块编译、函数调用、
 * 微任务执行等运行时环境。
 */

#pragma once

#include <iostream>
#include <memory>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/vm.h>
#include <mjs/job_queue.h>
#include <mjs/shape/shape.h>
#include <mjs/shape/shape_manager.h>
#include <mjs/gc/gc_manager.h>
#include <mjs/local_const_pool.h>

namespace mjs {

/**
 * @class Context
 * @brief JavaScript 执行上下文管理器
 *
 * 负责管理 JavaScript 代码的执行环境，包括：
 * - 模块编译和执行
 * - 函数调用栈管理
 * - 常量池管理
 * - 垃圾回收协调
 * - 微任务队列执行
 *
 * @note 每个 Context 实例都是独立的执行环境
 * @warning Context 不是线程安全的
 * @see Runtime 运行时环境
 * @see VM 虚拟机
 */

class Runtime;
class GCHandleScopeBase;

class Context : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 * @param runtime 运行时环境指针
	 * @throw std::invalid_argument 当 runtime 为 nullptr 时抛出
	 */
	Context(Runtime* runtime);

	/**
	 * @brief 析构函数
	 */
	~Context();

	/**
	 * @brief 编译 JavaScript 模块
	 *
	 * 将 JavaScript 源代码编译为内部字节码表示，并创建模块定义。
	 *
	 * @param module_name 模块名称
	 * @param script JavaScript 源代码
	 * @return 编译后的模块值
	 * @throw CompileError 编译错误时抛出
	 * @note 模块名称必须是唯一的
	 */
    Value CompileModule(std::string module_name, std::string_view script);

	/**
	 * @brief 执行已编译的模块
	 * @param value 模块值指针
	 * @return 模块执行结果值
	 * @throw RuntimeError 运行时错误时抛出
	 */
    Value CallModule(Value* value);

    /**
     * @brief 编译并执行 JavaScript 代码
     *
     * 将源代码编译为字节码并立即执行，适用于一次性脚本执行。
     *
     * @param module_name 模块名称
     * @param script JavaScript 源代码
     * @return 执行结果值
     * @throw CompileError 编译错误时抛出
     * @throw RuntimeError 运行时错误时抛出
     */
    Value Eval(std::string module_name, std::string_view script);

	/**
	 * @brief 调用 JavaScript 函数
	 *
	 * 使用迭代器范围作为参数调用指定的 JavaScript 函数。
	 *
	 * @tparam It 迭代器类型，必须满足随机访问迭代器要求
	 * @param func_val 要调用的函数值指针
	 * @param this_val this 上下文值
	 * @param begin 参数起始迭代器
	 * @param end 参数结束迭代器
	 * @return 函数执行结果
	 * @throw RuntimeError 运行时错误时抛出
	 */
	template<typename It>
	Value CallFunction(Value* func_val, Value this_val, It begin, It end);

	/**
	 * @brief 执行微任务队列
	 *
	 * 执行所有待处理的微任务，直到队列为空。
	 * 微任务包括 Promise 回调等异步操作。
	 */
	void ExecuteMicrotasks() {
		while (!microtask_queue_.empty()) {
			auto& task = microtask_queue_.front();
            CallFunction(&task.func(), task.this_val(), task.argv().begin(), task.argv().end());
			microtask_queue_.pop_front();
		}
	}

    /**
     * @brief 增加常量引用计数
     * @param const_index 常量索引
     */
    void ReferenceConstValue(ConstIndex const_index);

    /**
     * @brief 减少常量引用计数
     * @param const_index 常量索引
     * @note 当引用计数为0时自动回收常量
     */
    void DereferenceConstValue(ConstIndex const_index);

    /**
     * @brief 查找常量或在本地常量池中插入
     * @param value 常量值
     * @return 常量索引
     */
    ConstIndex FindConstOrInsertToLocal(const Value& value);

    /**
     * @brief 查找常量或在全局常量池中插入
     * @param value 常量值
     * @return 常量索引
     */
    ConstIndex FindConstOrInsertToGlobal(const Value& value);

    /**
     * @brief 获取常量值
     * @param const_index 常量索引
     * @return 常量值常量引用
     * @throw std::out_of_range 当索引超出范围时抛出
     */
    const Value& GetConstValue(ConstIndex const_index);

	/**
	 * @brief 获取运行时环境引用
	 * @return 运行时环境常量引用
	 */
	auto& runtime() const { return *runtime_; }

    /**
     * @brief 获取本地常量池引用
     * @return 本地常量池引用
     */
    auto& local_const_pool() { return local_const_pool_; }

	/**
	 * @brief 获取微任务队列常量引用
	 * @return 微任务队列常量引用
	 */
	const auto& microtask_queue() const { return microtask_queue_; }

	/**
	 * @brief 获取微任务队列引用
	 * @return 微任务队列引用
	 */
	auto& microtask_queue() { return microtask_queue_; }

    // auto& symbol_table() { return symbol_table_; }

    /**
     * @brief 获取形状管理器引用
     * @return 形状管理器引用
     */
    ShapeManager& shape_manager() { return shape_manager_; }

	/**
	 * @brief 获取垃圾回收管理器引用
	 * @return 垃圾回收管理器引用
	 */
	GCManager& gc_manager() { return gc_manager_; }

	/**
	 * @brief 获取虚拟机引用
	 * @return 虚拟机引用
	 */
	VM& vm() { return vm_; }

    /**
     * @brief 推入 HandleScope
     * @param scope HandleScope 指针
     */
    void PushHandleScope(GCHandleScopeBase* scope);

    /**
     * @brief 弹出 HandleScope
     */
    void PopHandleScope();

    /**
     * @brief 获取当前 HandleScope 栈顶
     * @return HandleScopeBase 指针，如果没有则返回 nullptr
     */
    GCHandleScopeBase* current_handle_scope() const {
        return current_handle_scope_;
    }

private:
    Runtime* runtime_;                    ///< 运行时环境指针
	LocalConstPool local_const_pool_;      ///< 本地常量池
	GCManager gc_manager_;                 ///< 垃圾回收管理器
	VM vm_;                                ///< 虚拟机实例
	JobQueue microtask_queue_;             ///< 微任务队列
    ShapeManager shape_manager_;          ///< 形状管理器
    GCHandleScopeBase* current_handle_scope_ = nullptr;  ///< 当前 HandleScope 栈顶
};

} // namespace mjs

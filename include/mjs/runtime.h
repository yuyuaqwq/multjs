/**
 * @file runtime.h
 * @brief JavaScript 运行时环境管理
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎的运行时环境，负责管理全局资源、
 * 常量池、垃圾回收、模块管理等共享组件。
 */

#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>
#include <mjs/class_def_table.h>
#include <mjs/module_manager.h>
#include <mjs/value.h>
#include <mjs/shape.h>
#include <mjs/gc_manager.h>
#include <mjs/error.h>

namespace mjs {

/**
 * @class Runtime
 * @brief JavaScript 运行时环境管理器
 *
 * 负责管理 JavaScript 引擎的全局资源和共享组件，包括：
 * - 全局常量池管理
 * - 垃圾回收协调
 * - 形状管理器（对象布局优化）
 * - 类定义表
 * - 模块管理器
 * - 全局 this 对象
 *
 * @note Runtime 是单例模式，每个进程应该只有一个实例
 * @warning Runtime 不是线程安全的，需要外部同步
 * @see Context 执行上下文
 * @see VM 虚拟机
 */
class Runtime : public noncopyable {
public:
	/**
	 * @brief 默认构造函数
	 * 使用默认的模块管理器初始化运行时环境
	 */
	Runtime();

	/**
	 * @brief 构造函数
	 * @param module_manager 自定义模块管理器
	 */
	Runtime(std::unique_ptr<ModuleManagerBase> module_manager);

	/**
	 * @brief 析构函数
	 */
	~Runtime();

	/**
	 * @brief 向全局 this 对象添加属性
	 * @param property_key 属性键名
	 * @param value 属性值
	 */
	void AddPropertyToGlobalThis(const char* property_key, Value&& value);

	/**
	 * @brief 获取全局常量池常量引用
	 * @return 全局常量池常量引用
	 */
	const auto& const_pool() const { return const_pool_; }

	/**
	 * @brief 获取全局常量池引用
	 * @return 全局常量池引用
	 */
	auto& const_pool() { return const_pool_; }

	/**
	 * @brief 获取垃圾回收管理器引用
	 * @return 垃圾回收管理器引用
	 */
	auto& gc_manager() { return gc_manager_; }

	/**
	 * @brief 获取线程本地栈引用
	 * @return 线程本地栈引用
	 * @note 每个线程有独立的栈实例
	 */
	auto& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

	/**
	 * @brief 获取形状管理器引用
	 * @return 形状管理器引用
	 */
	auto& shape_manager() { return shape_manager_; }

	/**
	 * @brief 获取全局 this 对象引用
	 * @return 全局 this 对象引用
	 */
	auto& global_this() { return global_this_; }

	/**
	 * @brief 获取类定义表常量引用
	 * @return 类定义表常量引用
	 */
	const auto& class_def_table() const { return class_def_table_; }

	/**
	 * @brief 获取类定义表引用
	 * @return 类定义表引用
	 */
	auto& class_def_table() { return class_def_table_; }

	/**
	 * @brief 获取模块管理器引用
	 * @return 模块管理器引用
	 */
	auto& module_manager() { return *module_manager_; }

private:
	/**
	 * @brief 初始化运行时环境
	 * 设置所有必要的组件和默认配置
	 */
	void Initialize();

	/**
	 * @brief 初始化全局 this 对象
	 * 创建并配置全局 this 对象的默认属性和方法
	 */
	void GlobalThisInitialize();

	/**
	 * @brief 初始化控制台功能
	 * 设置 console.log 等控制台相关功能
	 */
	void ConsoleInitialize();

private:
	GlobalConstPool const_pool_;              ///< 全局常量池
	GCManager gc_manager_;                    ///< 垃圾回收管理器
	ShapeManager shape_manager_;              ///< 形状管理器（对象布局优化）
	Value global_this_;                       ///< 全局 this 对象
	ClassDefTable class_def_table_;           ///< 类定义表
	std::unique_ptr<ModuleManagerBase> module_manager_; ///< 模块管理器
};

} // namespace mjs
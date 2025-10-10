/**
 * @file module_manager.h
 * @brief JavaScript 模块管理器系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的模块管理器系统，包括模块管理器基类
 * 和具体实现类，用于管理 C++ 模块和 JavaScript 模块的加载和缓存。
 */

#pragma once

#include <filesystem>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

/**
 * @class ModuleManagerBase
 * @brief 模块管理器基类
 *
 * 定义模块管理器的抽象接口，包括 C++ 模块添加、模块获取和缓存清理功能。
 * 继承自 noncopyable，确保不可拷贝。
 *
 * @see noncopyable 不可拷贝基类
 */
class ModuleManagerBase : public noncopyable {
public:
	/**
	 * @brief 添加 C++ 模块
	 * @param path 模块路径
	 * @param cpp_module_object C++ 模块对象指针
	 */
	virtual void AddCppModule(std::string_view path, CppModuleObject* cpp_module_object) = 0;

	/**
	 * @brief 获取模块
	 * @param ctx 执行上下文指针
	 * @param path 模块路径
	 * @return 模块值
	 */
	virtual Value GetModule(Context* ctx, std::string_view path) = 0;

	/**
	 * @brief 异步获取模块
	 * @param ctx 执行上下文指针
	 * @param path 模块路径
	 * @return 模块值
	 */
	virtual Value GetModuleAsync(Context* ctx, std::string_view path) = 0;

	/**
	 * @brief 清理模块缓存
	 */
	virtual void ClearModuleCache() = 0;
};

/**
 * @class ModuleManager
 * @brief 模块管理器实现类
 *
 * 实现 ModuleManagerBase 接口，提供具体的模块管理功能，
 * 包括 C++ 模块缓存和 JavaScript 模块缓存管理。
 *
 * @see ModuleManagerBase 模块管理器基类
 */
class ModuleManager : public ModuleManagerBase {
public:
	void AddCppModule(std::string_view path, CppModuleObject* cpp_module_object) override;
	Value GetModule(Context* ctx, std::string_view path) override;
	Value GetModuleAsync(Context* ctx, std::string_view path) override;
	void ClearModuleCache() override;

protected:
	std::unordered_map<std::filesystem::path, Value> cpp_module_cache_; ///< C++ 模块缓存
	std::unordered_map<std::filesystem::path, Value> module_cache_;     ///< JavaScript 模块缓存
};

} // namespace mjs
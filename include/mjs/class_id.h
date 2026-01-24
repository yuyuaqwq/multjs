/**
 * @file class_def.h
 * @brief JavaScript 类ID
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <cstdint>

namespace mjs {

/**
 * @enum ClassId
 * @brief 类标识符枚举
 *
 * 定义了 JavaScript 引擎支持的所有内置类类型标识符。
 */
enum class ClassId : uint16_t {
	kInvalid = 0,          ///< 无效类标识符
	kObject,               ///< 普通对象类
	kFunctionObject,       ///< 函数对象类
	kNumberObject,         ///< 数字对象类
	kStringObject,         ///< 字符串对象类
	kArrayObject,          ///< 数组对象类
	kGeneratorObject,      ///< 生成器对象类
	kPromiseObject,        ///< Promise 对象类
	kAsyncObject,          ///< 异步对象类
	kModuleObject,         ///< 模块对象类
	kCppModuleObject,      ///< C++ 模块对象类
	kSymbol,               ///< Symbol 类

	kCustom,               ///< 自定义类标识符
};

} // namespace mjs
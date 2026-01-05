/**
 * @file class_def.h
 * @brief JavaScript 类定义系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的类定义系统，包括对象内部方法枚举、
 * 函数内部方法枚举、类标识符枚举和类定义基类。
 */

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/error.h>
#include <mjs/value.h>

namespace mjs {

/**
 * @enum ObjectInternalMethods
 * @brief 对象内部方法枚举
 *
 * 定义了 JavaScript 对象支持的所有内部方法，用于实现对象的行为规范。
 * 这些方法对应于 ECMAScript 规范中的内部方法。
 */
enum class ObjectInternalMethods {
	kGetPrototypeOf = 1 << 0,     ///< [[GetPrototypeOf]] 内部方法
	kSetPrototypeOf = 1 << 1,     ///< [[SetPrototypeOf]] 内部方法
	kIsExtensible = 1 << 2,       ///< [[IsExtensible]] 内部方法
	kPreventExtensions = 1 << 3,  ///< [[PreventExtensions]] 内部方法
	kGetOwnProperty = 1 << 4,     ///< [[GetOwnProperty]] 内部方法
	kDefineOwnProperty = 1 << 5,  ///< [[DefineOwnProperty]] 内部方法
	kHasProperty = 1 << 6,        ///< [[HasProperty]] 内部方法
	kGet = 1 << 7,                ///< [[Get]] 内部方法
	kSet = 1 << 8,                ///< [[Set]] 内部方法
	kDelete = 1 << 9,             ///< [[Delete]] 内部方法
	kOwnPropertyKeys = 1 << 10,   ///< [[OwnPropertyKeys]] 内部方法
};

/**
 * @enum FunctionInternalMethods
 * @brief 函数内部方法枚举
 *
 * 定义了 JavaScript 函数支持的所有内部方法。
 */
enum class FunctionInternalMethods {
	kCall = 1 << 1,               ///< [[Call]] 内部方法
};


/**
 * @enum ClassId
 * @brief 类标识符枚举
 *
 * 定义了 JavaScript 引擎支持的所有内置类类型标识符。
 */
enum class ClassId : uint16_t {
	kInvalid = 0,          ///< 无效类标识符
	kSymbol,               ///< Symbol 类
	kObject,               ///< 普通对象类
	kNumberObject,         ///< 数字对象类
	kStringObject,         ///< 字符串对象类
	kArrayObject,          ///< 数组对象类
	kFunctionObject,       ///< 函数对象类
	kGeneratorObject,      ///< 生成器对象类
	kPromiseObject,        ///< Promise 对象类
	kAsyncObject,          ///< 异步对象类
	kModuleObject,         ///< 模块对象类
	kConstructorObject,    ///< 构造函数对象类
	kCppModuleObject,      ///< C++ 模块对象类

	kCustom,               ///< 自定义类标识符
};

class Runtime;

/**
 * @class ClassDef
 * @brief 类定义基类
 *
 * 所有 JavaScript 类定义的基类，提供以下核心功能：
 * - 类标识符管理
 * - 构造函数支持
 * - 原型对象管理
 * - 类名称管理
 *
 * @note 继承自 noncopyable 确保单例特性
 * @see Object 对象基类
 */
class ClassDef : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 * @param runtime 运行时环境指针
	 * @param id 类标识符
	 * @param name 类名称
	 */
	ClassDef(Runtime* runtime, ClassId id, const char* name);

	/**
	 * @brief 虚析构函数
	 */
	virtual ~ClassDef();

	/**
	 * @brief 新建构造函数
	 *
	 * 如果允许通过 new 构造，需要在派生类中重写此函数，如 new ArrayObject()。
	 *
	 * @param context 执行上下文指针
	 * @param par_count 参数数量
	 * @param stack 栈帧引用
	 * @return 新创建的对象值
	 * @throw InternalError 当类不支持 new 构造时抛出
	 */
	virtual Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
		throw InternalError(
			"This constructor cannot be called with 'new'. "
			"Either this is not a constructible function, "
			"or you need to override NewConstructor() in the derived class."
		);
	}

	/**
	 * @brief 获取类标识符
	 * @return 类标识符
	 */
	ClassId id() const { return id_; }

	/**
	 * @brief 获取类名称常量索引
	 * @return 类名称常量索引
	 */
	const auto& name() const { return name_; }

	/**
	 * @brief 获取类名称字符串
	 * @return 类名称字符串常量引用
	 */
	const auto& name_string() const { return name_string_; }

	/**
	 * @brief 获取原型对象
	 * @return 原型对象常量引用
	 */
	const Value& prototype() const { return prototype_; }

	/**
	 * @brief 获取指定类型的类定义引用
	 * @tparam ClassDefT 类定义类型
	 * @return 指定类型的类定义引用
	 */
	template<typename ClassDefT>
	ClassDefT& get() {
		return static_cast<ClassDefT&>(*this);
	}

protected:
	ClassId id_;                    ///< 类标识符
	ConstIndex name_;               ///< 类名称常量索引
	std::string name_string_;       ///< 类名称字符串

	Value constructor_object_;      ///< 构造函数对象
	Value prototype_;               ///< 原型对象
};

/**
 * @brief 类定义唯一指针类型别名
 */
using ClassDefUnique = std::unique_ptr<ClassDef>;

} // namespace mjs
/**
 * @file function_def.h
 * @brief JavaScript 函数定义系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的函数定义系统，包括函数定义基类和
 * 具体函数定义类，支持普通函数、箭头函数、生成器函数等多种函数类型。
 */

#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/reference_counter.h>
#include <mjs/exception.h>
#include <mjs/variable.h>
#include <mjs/bytecode_table.h>
#include <mjs/closure.h>
#include <mjs/debug.h>

namespace mjs {

class ModuleDef;

/**
 * @class FunctionDefBase
 * @brief 函数定义基类
 *
 * 所有函数定义的基类，提供以下核心功能：
 * - 函数类型标识（普通函数、箭头函数、生成器函数等）
 * - 字节码表管理
 * - 变量定义表管理
 * - 闭包变量表管理
 * - 异常处理表管理
 * - 调试信息管理
 *
 * @note 不会有循环引用问题，仅使用引用计数管理
 * @see FunctionDef 具体函数定义类
 * @see ModuleDef 模块定义类
 */
class FunctionDefBase {
public:
	/**
	 * @brief 反汇编函数字节码
	 * @param context 执行上下文指针
	 * @return 反汇编后的字符串表示
	 */
	std::string Disassembly(Context* context) const;

	/**
	 * @brief 获取所属模块定义
	 * @return 模块定义常量引用
	 */
	const auto& module_def() const { return *module_def_; }

	/**
	 * @brief 获取所属模块可变定义
	 * @return 模块定义常量引用
	 */
	auto& module_def() { return *module_def_; }

	/**
	 * @brief 获取函数名称
	 * @return 函数名称常量引用
	 */
	const auto& name() const { return name_; }

	/**
	 * @brief 设置为普通函数类型
	 */
	void set_is_normal() {
		flags_.is_normal_ = true;
		flags_.is_module_ = false;
		flags_.is_arrow_ = false;
	}

	/**
	 * @brief 设置为模块函数类型
	 */
	void set_is_module() {
		flags_.is_normal_ = false;
		flags_.is_module_ = true;
		flags_.is_arrow_ = false;
	}

	/**
	 * @brief 设置为箭头函数类型
	 */
	void set_is_arrow() {
		flags_.is_normal_ = false;
		flags_.is_module_ = false;
		flags_.is_arrow_ = true;
	}

	/**
	 * @brief 设置为生成器函数类型
	 */
	void set_is_generator() {
		flags_.is_generator_ = true;
	}

	/**
	 * @brief 设置为异步函数类型
	 */
	void set_is_async() {
		flags_.is_asnyc_ = true;
	}

	/**
	 * @brief 检查是否为普通函数
	 * @return 是否为普通函数
	 */
	bool is_normal() const {
		return flags_.is_normal_;
	}

	/**
	 * @brief 检查是否为模块函数
	 * @return 是否为模块函数
	 */
	bool is_module() const {
		return flags_.is_module_;
	}

	/**
	 * @brief 检查是否为箭头函数
	 * @return 是否为箭头函数
	 */
	bool is_arrow() const {
		return flags_.is_arrow_;
	}

	/**
	 * @brief 检查是否为生成器函数
	 * @return 是否为生成器函数
	 */
	bool is_generator() const {
		return flags_.is_generator_;
	}

	/**
	 * @brief 检查是否为异步函数
	 * @return 是否为异步函数
	 */
	bool is_async() const {
		return flags_.is_asnyc_;
	}

	/**
	 * @brief 获取参数数量
	 * @return 参数数量
	 */
	auto param_count() const { return param_count_; }

	/**
	 * @brief 获取字节码表常量引用
	 * @return 字节码表常量引用
	 */
	const auto& bytecode_table() const { return bytecode_table_; }

	/**
	 * @brief 获取字节码表引用
	 * @return 字节码表引用
	 */
	auto& bytecode_table() { return bytecode_table_; }

	/**
	 * @brief 获取变量定义表常量引用
	 * @return 变量定义表常量引用
	 */
	const auto& var_def_table() const { return var_def_table_; }

	/**
	 * @brief 获取变量定义表引用
	 * @return 变量定义表引用
	 */
	auto& var_def_table() { return var_def_table_; }

	/**
	 * @brief 获取闭包变量表常量引用
	 * @return 闭包变量表常量引用
	 */
	const auto& closure_var_table() const { return closure_var_table_; }

	/**
	 * @brief 获取闭包变量表引用
	 * @return 闭包变量表引用
	 */
	auto& closure_var_table() { return closure_var_table_; }

	/**
	 * @brief 检查是否包含 this 参数
	 * @return 是否包含 this 参数
	 */
	const auto& has_this() const { return has_this_; }

	/**
	 * @brief 设置是否包含 this 参数
	 * @param has_this 是否包含 this 参数
	 */
	void set_has_this(bool has_this) { has_this_ = has_this; }

	/**
	 * @brief 获取异常处理表常量引用
	 * @return 异常处理表常量引用
	 */
	const auto& exception_table() const { return exception_table_; }

	/**
	 * @brief 获取异常处理表引用
	 * @return 异常处理表引用
	 */
	auto& exception_table() { return exception_table_; }

	/**
	 * @brief 获取调试信息表常量引用
	 * @return 调试信息表常量引用
	 */
	const auto& debug_table() const { return debug_table_; }

	/**
	 * @brief 获取调试信息表引用
	 * @return 调试信息表引用
	 */
    auto& debug_table() { return debug_table_; }

protected:
	/**
	 * @brief 受保护构造函数
	 * @param module_def 所属模块定义指针
	 * @param name 函数名称
	 * @param param_count 参数数量
	 */
	FunctionDefBase(ModuleDef* module_def, std::string name, uint32_t param_count) noexcept;

protected:
	ModuleDef* module_def_;                ///< 所属模块定义指针

	std::string name_;                     ///< 函数名称
	// FunctionType type_ = FunctionType::kNormal;  ///< 函数类型（已注释）

	struct {
		uint32_t is_normal_ : 1 = 0;         ///< 是否为普通函数标记
		uint32_t is_module_ : 1 = 0;         ///< 是否为模块函数标记
		uint32_t is_arrow_ : 1 = 0;          ///< 是否为箭头函数标记
		uint32_t is_generator_ : 1 = 0;      ///< 是否为生成器函数标记
		uint32_t is_asnyc_ : 1 = 0;          ///< 是否为异步函数标记
	} flags_;

	uint32_t param_count_;                 ///< 参数数量

	BytecodeTable bytecode_table_;         ///< 字节码表

	VarDefTable var_def_table_;            ///< 变量定义表

	ClosureVarTable closure_var_table_;    ///< 闭包变量表

	bool has_this_ = false;                ///< 是否包含 this 参数

	ExceptionTable exception_table_;       ///< 异常处理表

	DebugTable debug_table_;               ///< 调试信息表
};

/**
 * @class FunctionDef
 * @brief 具体函数定义类
 *
 * 继承自 FunctionDefBase 和 ReferenceCounter，提供引用计数管理的
 * 具体函数定义实现。支持自动内存管理和垃圾回收。
 *
 * @see FunctionDefBase 函数定义基类
 * @see ReferenceCounter 引用计数基类
 */
class FunctionDef : public ReferenceCounter<FunctionDef>, public FunctionDefBase {
public:
	/**
	 * @brief 创建新的函数定义
	 * @param module_def 所属模块定义指针
	 * @param name 函数名称
	 * @param param_count 参数数量
	 * @return 新创建的函数定义指针
	 */
	static FunctionDef* New(ModuleDef* module_def, std::string name, uint32_t param_count) {
		return new FunctionDef(module_def, std::move(name), param_count);
	}

private:
	using FunctionDefBase::FunctionDefBase;  ///< 继承基类构造函数

};

} // namespace mjs


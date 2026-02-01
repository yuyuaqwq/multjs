/**
 * @file module_def.h
 * @brief JavaScript 模块定义系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的模块定义系统，继承自函数定义基类，
 * 提供模块级别的编译和执行支持，包括源代码管理和导出变量管理。
 */

#pragma once

#include <mjs/value/function_def.h>
#include <mjs/line_table.h>

namespace mjs {

class Runtime;

/**
 * @class ModuleDef
 * @brief JavaScript 模块定义类
 *
 * 继承自 FunctionDefBase 和 ReferenceCounter，提供模块级别的
 * 编译和执行支持，包括：
 * - 源代码管理
 * - 导出变量定义表管理
 * - 行号表管理（用于调试）
 *
 * @note 不会有循环引用问题，仅使用引用计数管理
 * @see FunctionDefBase 函数定义基类
 * @see ReferenceCounter 引用计数基类
 */
class ModuleDef : public ReferenceCounter<ModuleDef>, public FunctionDefBase {
public:
	/**
	 * @brief 创建新的模块定义
	 * @param runtime 运行时环境指针
	 * @param name 模块名称
	 * @param source 模块源代码
	 * @param param_count 参数数量
	 * @return 新创建的模块定义指针
	 */
	static ModuleDef* New(Runtime* runtime, std::string name, std::string_view source, uint32_t param_count) {
		return new ModuleDef(runtime, std::move(name), source, param_count);
	}

	/**
	 * @brief 获取导出变量定义表常量引用
	 * @return 导出变量定义表常量引用
	 */
	const auto& export_var_def_table() const { return export_var_def_table_; }

	/**
	 * @brief 获取导出变量定义表引用
	 * @return 导出变量定义表引用
	 */
	auto& export_var_def_table() { return export_var_def_table_; }

	/**
	 * @brief 获取行号表常量引用
	 * @return 行号表常量引用
	 * @note 行号表用于调试和错误定位
	 */
	const auto& line_table() const { return line_table_; }

private:
	/**
	 * @brief 私有构造函数
	 * @param runtime 运行时环境指针
	 * @param name 模块名称
	 * @param source 模块源代码
	 * @param param_count 参数数量
	 */
	ModuleDef(Runtime* runtime, std::string name, std::string_view source, uint32_t param_count)
		: runtime_(runtime)
		, FunctionDefBase(this, name, param_count)
	{
		line_table_.Build(source);
	}

private:
	Runtime* runtime_;                     ///< 运行时环境指针

	ExportVarDefTable export_var_def_table_; ///< 导出变量定义表
	LineTable line_table_;                 ///< 行号表（用于调试和错误定位）
};

} // namespace mjs


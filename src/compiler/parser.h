/**
 * @file parser.h
 * @brief JavaScript 语法分析器定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的语法分析器，负责将词法标记序列
 * 转换为抽象语法树（AST），支持完整的 JavaScript 语法特性，包括：
 * - 表达式解析（赋值、函数、运算符、字面量等）
 * - 语句解析（变量声明、控制流、函数声明等）
 * - 模块语法（import/export）
 * - 异步和生成器函数
 * - 模板字符串和类型注解
 * - 错误处理和语法验证
 */

#pragma once

#include <exception>
#include <memory>
#include <vector>
#include <optional>
#include <string_view>

#include <mjs/noncopyable.h>
#include <mjs/token.h>

#include "src/compiler/lexer.h"
#include "src/compiler/expression.h"
#include "src/compiler/statement_impl/type_annotation.h"
#include "src/compiler/statement_impl/union_type.h"
#include "src/compiler/statement_impl/import_declaration.h"

namespace mjs {
namespace compiler {

/**
 * @brief 测试命名空间，包含解析器测试相关的类
 */
namespace test {
/**
 * @class ParserTest
 * @brief 解析器测试类，用于单元测试
 */
class ParserTest;
} // namespace test

/**
 * @class Parser
 * @brief 语法分析器，负责将词法标记序列转换为抽象语法树
 * 
 * 该类实现了一个递归下降的语法分析器，用于解析JavaScript语言的语法结构
 */
class Parser : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 * @param lexer 词法分析器指针
	 */
	explicit Parser(Lexer* lexer);

	/**
	 * @brief 解析整个程序
	 * @throws SyntaxError 如果解析过程中遇到语法错误
	 */
	void ParseProgram();

	/**
	 * @brief 获取解析后的语句列表
	 * @return 语句列表的常量引用
	 */
	[[nodiscard]] const auto& statements() const noexcept { return statements_; }

	/**
	 * @brief 获取导入声明列表
	 * @return 导入声明列表的常量引用
	 */
	[[nodiscard]] const auto& import_declarations() const noexcept { return import_declarations_; }

private:
	/**
	 * @brief 解析函数参数列表
	 * @return 参数名称列表
	 */
	std::vector<std::string> ParseParameters();
	
	/**
	 * @brief 解析表达式列表
	 * @param begin 开始标记类型
	 * @param end 结束标记类型
	 * @param allow_comma_end 是否允许逗号结尾
	 * @return 表达式列表
	 */
	std::vector<std::unique_ptr<Expression>> ParseExpressions(TokenType begin, TokenType end, bool allow_comma_end);

private:
	/**
	 * @brief 声明友元类，使其可以访问私有成员
	 * @details 允许测试类访问解析器的内部状态进行单元测试
	 */
	friend class ::mjs::compiler::test::ParserTest;

	Lexer* lexer_;  ///< 词法分析器指针，用于获取词法标记

	std::vector<std::unique_ptr<Statement>> statements_;  ///< 解析后的语句列表，存储整个程序的抽象语法树
	std::vector<std::unique_ptr<ImportDeclaration>> import_declarations_;  ///< 导入声明列表，用于模块系统
};

} // namespace compiler
} // namespace mjs
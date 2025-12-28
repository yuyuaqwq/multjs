/**
 * @file expression_base.h
 * @brief 表达式基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/source_define.h>

#include <mjs/token.h>

namespace mjs {

class FunctionDefBase;

namespace compiler {

class Lexer;
class CodeGenerator;

/**
 * @enum ValueCategory
 * @brief 表达式值类别
 */
enum class ValueCategory {
    kLValue, ///< 左值（可以出现在赋值语句左侧）
    kRValue, ///< 右值（只能出现在赋值语句右侧）
};

/**
 * @class Expression
 * @brief 表达式基类，所有表达式节点的抽象基类
 */
class Expression : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     */
    Expression(SourcePosition start, SourcePosition end)
        : start_(start), end_(end) {}

    /**
     * @brief 析构函数
     */
    virtual ~Expression() = default;

    /**
     * @brief 将表达式转换为指定类型
     * @tparam T 目标类型
     * @return 转换后的表达式引用
     */
    template<typename T>
    T& as() {
        return *static_cast<T*>(this);
    }

    /**
     * @brief 将表达式转换为指定类型（常量版本）
     * @tparam T 目标类型
     * @return 转换后的表达式常量引用
     */
    template<typename T>
    const T& as() const {
        return *static_cast<const T*>(this);
    }

    /**
     * @brief 获取值类别
     * @return 值类别
     */
    ValueCategory value_category() const { return value_category_; }

    /**
     * @brief 设置值类别
     * @param category 值类别
     */
    void set_value_category(ValueCategory category) {
        value_category_ = category;
    }

    /**
     * @brief 获取表达式起始位置
     * @return 起始位置
     */
    SourcePosition start() const { return start_; }

    /**
     * @brief 获取表达式结束位置
     * @return 结束位置
     */
    SourcePosition end() const { return end_; }

	/**
	 * @brief 解析表达式
	 * @param lexer 词法分析器
	 * @return 解析后的表达式
	 */
	static std::unique_ptr<Expression> ParseExpression(Lexer* lexer);

	/**
	 * @brief 解析new表达式、import表达式或成员表达式
	 *
	 * 这个函数处理以下几种表达式：
	 * - new表达式：new Constructor(...)
	 * - import表达式：import(...)
	 * - 成员表达式：object.property, object[property]
	 *
	 * @param lexer 词法分析器
	 * @return 解析后的表达式
	 */
	static std::unique_ptr<Expression> ParseExpressionAtLeftHandSideLevel(Lexer* lexer);

	/**
	 * @brief 解析函数参数列表
	 * @param lexer 词法分析器
	 * @return 参数名称列表
	 */
	static std::vector<std::string> ParseParameters(Lexer* lexer);

	/**
	 * @brief 解析表达式列表
	 * @param lexer 词法分析器
	 * @param begin 开始标记类型
	 * @param end 结束标记类型
	 * @param allow_comma_end 是否允许逗号结尾
	 * @return 表达式列表
	 */
	static std::vector<std::unique_ptr<Expression>> ParseExpressions(Lexer* lexer, TokenType begin, TokenType end, bool allow_comma_end);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    virtual void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const = 0;

private:
    ValueCategory value_category_ = ValueCategory::kRValue; ///< 值类别
    SourcePosition start_; ///< 表达式起始位置
    SourcePosition end_;   ///< 表达式结束位置
};

} // namespace compiler
} // namespace mjs
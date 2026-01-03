/**
 * @file call_expression.h
 * @brief 函数调用表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>

#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @class CallExpression
 * @brief 函数调用表达式
 */
class CallExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param callee 被调用表达式
     * @param arguments 参数表达式列表
     */
    CallExpression(SourceBytePosition start, SourceBytePosition end,
                std::unique_ptr<Expression> callee,
                std::vector<std::unique_ptr<Expression>>&& arguments)
        : Expression(start, end), callee_(std::move(callee)),
        arguments_(std::move(arguments)) {}


    /**
     * @brief 获取被调用表达式
     * @return 被调用表达式的常量引用
     */
    const std::unique_ptr<Expression>& callee() const { return callee_; }

    /**
     * @brief 获取参数表达式列表
     * @return 参数表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析成员表达式或调用表达式
     *
     * 这个函数处理以下几种表达式：
     * - 成员访问：object.property
     * - 计算属性：object[property]
     * - 函数调用：function(args)
     * - 可选链：object?.property, object?.[property], function?.()
     *
     * @param lexer 词法分析器
     * @param right 右侧表达式
     * @param match_lparen 是否需要匹配左括号（用于嵌套调用）
     * @return 解析后的表达式
     */
    static std::unique_ptr<Expression> ParseExpressionAtCallLevel(
        Lexer* lexer, std::unique_ptr<Expression> right, bool match_lparen);

    /**
     * @brief 解析函数调用表达式
     *
     * 函数调用表达式的形式为：callee(arg1, arg2, ...)
     *
     * @param lexer 词法分析器
     * @param callee 被调用的函数表达式
     * @return 解析后的调用表达式
     */
    static std::unique_ptr<CallExpression> ParseCallExpression(Lexer* lexer, std::unique_ptr<Expression> callee);

private:
    std::unique_ptr<Expression> callee_; ///< 被调用表达式
    std::vector<std::unique_ptr<Expression>> arguments_; ///< 参数表达式列表
};

} // namespace compiler
} // namespace mjs
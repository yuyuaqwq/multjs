/**
 * @file unary_expression.h
 * @brief 一元表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class UnaryExpression
 * @brief 一元表达式
 */
class UnaryExpression : public Expression {
public:
    /**
     * @brief 构造一元表达式
     *
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param op 运算符类型
     * @param argument 操作数表达式
     * @param is_prefix 是否为前缀运算符（true为前缀，false为后缀）
     */
    UnaryExpression(SourcePosition start, SourcePosition end,
                TokenType op, std::unique_ptr<Expression> argument,
                bool is_prefix)
        : Expression(start, end), operator_(op), argument_(std::move(argument)),
          is_prefix_(is_prefix) {}


    /**
     * @brief 获取运算符类型
     * @return 运算符类型
     */
    TokenType op() const { return operator_; }

    /**
     * @brief 获取操作数表达式
     * @return 操作数表达式的常量引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }

    /**
     * @brief 判断是否为前缀运算符
     * @return 如果是前缀运算符则返回true，否则返回false
     */
    bool is_prefix() const { return is_prefix_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析一元表达式
     * @param lexer 词法分析器
     * @return 解析后的一元表达式
     */
    static std::unique_ptr<Expression> ParseExpressionAtUnaryLevel(Lexer* lexer);

    /**
     * @brief 解析后缀表达式
     * @param lexer 词法分析器
     * @return 解析后的后缀表达式
     */
    static std::unique_ptr<Expression> ParsePostfixExpression(Lexer* lexer);

private:
    TokenType operator_;                   ///< 运算符类型
    std::unique_ptr<Expression> argument_; ///< 操作数表达式
    bool is_prefix_;                       ///< 是否为前缀运算符
};

} // namespace compiler
} // namespace mjs
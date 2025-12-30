/**
 * @file binary_expression.h
 * @brief 二元表达式
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
 * @class BinaryExpression
 * @brief 二元表达式
 */
class BinaryExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param op 运算符类型
     * @param left 左操作数表达式
     * @param right 右操作数表达式
     */
    BinaryExpression(SourcePosition start, SourcePosition end,
                TokenType op, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
        : Expression(start, end), operator_(op), left_(std::move(left)), right_(std::move(right)) {}


    /**
     * @brief 获取运算符类型
     * @return 运算符类型
     */
    TokenType op() const { return operator_; }

    /**
     * @brief 获取左操作数表达式
     * @return 左操作数表达式的常量引用
     */
    const std::unique_ptr<Expression>& left() const { return left_; }

    /**
     * @brief 获取右操作数表达式
     * @return 右操作数表达式的常量引用
     */
    const std::unique_ptr<Expression>& right() const { return right_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析逗号优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtCommaLevel(Lexer* lexer);

    /**
     * @brief 解析逻辑或优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtLogicalOrLevel(Lexer* lexer);

    /**
     * @brief 解析逻辑与优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtLogicalAndLevel(Lexer* lexer);

    /**
     * @brief 解析按位或优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtBitwiseOrLevel(Lexer* lexer);

    /**
     * @brief 解析按位异或优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtBitwiseXorLevel(Lexer* lexer);

    /**
     * @brief 解析按位与优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtBitwiseAndLevel(Lexer* lexer);

    /**
     * @brief 解析相等性优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtEqualityLevel(Lexer* lexer);

    /**
     * @brief 解析关系优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtRelationalLevel(Lexer* lexer);

    /**
     * @brief 解析移位优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtShiftLevel(Lexer* lexer);

    /**
     * @brief 解析加法优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtAdditiveLevel(Lexer* lexer);

    /**
     * @brief 解析乘法优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtMultiplicativeLevel(Lexer* lexer);

    /**
     * @brief 解析幂运算优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtExponentiationLevel(Lexer* lexer);

private:
    TokenType operator_; ///< 运算符类型
    std::unique_ptr<Expression> left_; ///< 左操作数表达式
    std::unique_ptr<Expression> right_; ///< 右操作数表达式
};

} // namespace compiler
} // namespace mjs
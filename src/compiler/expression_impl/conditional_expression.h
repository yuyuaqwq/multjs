/**
 * @file conditional_expression.h
 * @brief 条件表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>
#include "../expression.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class ConditionalExpression
 * @brief 条件表达式
 */
class ConditionalExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param test 条件测试表达式
     * @param consequent 条件为真时的表达式
     * @param alternate 条件为假时的表达式
     */
    ConditionalExpression(SourcePosition start, SourcePosition end,
                std::unique_ptr<Expression> test,
                std::unique_ptr<Expression> consequent,
                std::unique_ptr<Expression> alternate)
        : Expression(start, end), test_(std::move(test)),
        consequent_(std::move(consequent)), alternate_(std::move(alternate)) {}


    /**
     * @brief 获取条件测试表达式
     * @return 条件测试表达式的常量引用
     */
    const std::unique_ptr<Expression>& test() const { return test_; }

    /**
     * @brief 获取条件为真时的表达式
     * @return 条件为真时表达式的常量引用
     */
    const std::unique_ptr<Expression>& consequent() const { return consequent_; }

    /**
     * @brief 获取条件为假时的表达式
     * @return 条件为假时表达式的常量引用
     */
    const std::unique_ptr<Expression>& alternate() const { return alternate_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析条件优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtConditionalLevel(Lexer* lexer);

private:
    std::unique_ptr<Expression> test_; ///< 条件测试表达式
    std::unique_ptr<Expression> consequent_; ///< 条件为真时的表达式
    std::unique_ptr<Expression> alternate_; ///< 条件为假时的表达式
};

} // namespace compiler
} // namespace mjs
/**
 * @file array_expression.h
 * @brief 数组表达式
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
 * @class ArrayExpression
 * @brief 数组表达式
 */
class ArrayExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param elements 数组元素表达式列表
     */
    ArrayExpression(SourceBytePosition start, SourceBytePosition end,
                std::vector<std::unique_ptr<Expression>>&& elements)
        : Expression(start, end), elements_(std::move(elements)) {}

    /**
     * @brief 获取数组元素表达式列表
     * @return 元素表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& elements() const { return elements_; }

    /**
     * @brief 解析数组表达式
     *
     * 数组表达式的形式为：[element1, element2, ...]
     * 支持稀疏数组（有空洞的数组）和展开运算符
     *
     * @param lexer 词法分析器
     * @return 解析后的数组表达式
     */
    static std::unique_ptr<ArrayExpression> ParseArrayExpression(Lexer* lexer);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::vector<std::unique_ptr<Expression>> elements_; ///< 数组元素表达式列表
};

} // namespace compiler
} // namespace mjs
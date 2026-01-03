/**
 * @file float_literal.h
 * @brief 浮点数字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "src/compiler/expression_impl/primary_expression.h"

namespace mjs {
namespace compiler {

/**
 * @class FloatLiteral
 * @brief 浮点数字面量表达式
 */
class FloatLiteral : public PrimaryExpression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param value 浮点数值
     */
    FloatLiteral(SourceBytePosition start, SourceBytePosition end, double value)
        : PrimaryExpression(start, end), value_(value) {}


    /**
     * @brief 获取浮点数值
     * @return 浮点数值
     */
    double value() const { return value_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    double value_; ///< 浮点数值
};

} // namespace compiler
} // namespace mjs
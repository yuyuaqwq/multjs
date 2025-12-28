/**
 * @file integer_literal.h
 * @brief 整数字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "primary_expression.h"

namespace mjs {
namespace compiler {

/**
 * @class IntegerLiteral
 * @brief 整数字面量表达式
 */
class IntegerLiteral : public PrimaryExpression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param value 整数值
     */
    IntegerLiteral(SourcePosition start, SourcePosition end, int64_t value)
        : PrimaryExpression(start, end), value_(value) {}


    /**
     * @brief 获取整数值
     * @return 整数值
     */
    int64_t value() const { return value_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    int64_t value_; ///< 整数值
};

} // namespace compiler
} // namespace mjs
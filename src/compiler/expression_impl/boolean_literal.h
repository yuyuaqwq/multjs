/**
 * @file boolean_literal.h
 * @brief 布尔字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "primary_expression.h"

namespace mjs {
namespace compiler {

/**
 * @class BooleanLiteral
 * @brief 布尔字面量表达式
 */
class BooleanLiteral : public PrimaryExpression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param value 布尔值
     */
    BooleanLiteral(SourceBytePosition start, SourceBytePosition end, bool value)
        : PrimaryExpression(start, end), value_(value) {}


    /**
     * @brief 获取布尔值
     * @return 布尔值
     */
    bool value() const { return value_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    bool value_; ///< 布尔值
};

} // namespace compiler
} // namespace mjs
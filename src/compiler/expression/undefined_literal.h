/**
 * @file undefined_literal.h
 * @brief undefined 字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "primary_expression.h"

namespace mjs {
namespace compiler {

/**
 * @class UndefinedLiteral
 * @brief undefined 字面量表达式
 */
class UndefinedLiteral : public PrimaryExpression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     */
    UndefinedLiteral(SourcePosition start, SourcePosition end)
        : PrimaryExpression(start, end) {}

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;
};

} // namespace compiler
} // namespace mjs
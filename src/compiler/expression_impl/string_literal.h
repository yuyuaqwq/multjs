/**
 * @file string_literal.h
 * @brief 字符串字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>

#include "src/compiler/expression_impl/primary_expression.h"

namespace mjs {
namespace compiler {

/**
 * @class StringLiteral
 * @brief 字符串字面量表达式
 */
class StringLiteral : public PrimaryExpression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param value 字符串值
     */
    StringLiteral(SourceBytePosition start, SourceBytePosition end, std::string&& value)
        : PrimaryExpression(start, end), value_(std::move(value)) {}

    /**
     * @brief 获取字符串值
     * @return 字符串值的常量引用
     */
    const std::string& value() const { return value_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string value_; ///< 字符串值
};

} // namespace compiler
} // namespace mjs
/**
 * @file string_literal.h
 * @brief 字符串字面量表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>

#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @class PrimaryExpression
 * @brief 基本表达式
 */
class PrimaryExpression : public Expression {
public:
    /**
     * @brief 构造函数
     */
    using Expression::Expression;

    static std::unique_ptr<Expression> ParsePrimaryExpression(Lexer* lexer);
};

} // namespace compiler
} // namespace mjs
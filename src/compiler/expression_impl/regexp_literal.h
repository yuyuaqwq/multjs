/**
 * @file regexp_literal.h
 * @brief 正则表达式字面量
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @class RegExpLiteral
 * @brief 正则表达式字面量表达式
 *
 * 表示JavaScript中的正则表达式字面量，如 /abc/g
 */
class RegExpLiteral : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 源代码起始位置
     * @param end 源代码结束位置
     * @param pattern 正则表达式模式
     * @param flags 正则表达式标志
     */
    RegExpLiteral(SourceBytePosition start, SourceBytePosition end,
                 std::string pattern, std::string flags)
        : Expression(start, end),
          pattern_(std::move(pattern)), flags_(std::move(flags)) {}

    /**
     * @brief 析构函数
     */
    ~RegExpLiteral() override = default;

    /**
     * @brief 获取模式字符串
     */
    const std::string& pattern() const { return pattern_; }

    /**
     * @brief 获取标志字符串
     */
    const std::string& flags() const { return flags_; }

    /**
     * @brief 生成代码
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string pattern_;  ///< 正则表达式模式
    std::string flags_;    ///< 正则表达式标志
};

} // namespace compiler
} // namespace mjs

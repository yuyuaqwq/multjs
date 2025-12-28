/**
 * @file template_literal.h
 * @brief 模板字符串表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class TemplateLiteral
 * @brief 模板字符串表达式
 */
class TemplateLiteral : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param expressions 模板表达式列表
     */
    TemplateLiteral(SourcePosition start, SourcePosition end,
        std::vector<std::unique_ptr<Expression>>&& expressions)
        : Expression(start, end), expressions_(std::move(expressions)) {}


    /**
     * @brief 获取模板表达式列表
     * @return 表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& expressions() const { return expressions_; }

    static std::unique_ptr<TemplateLiteral> ParseTemplateLiteral(Lexer* lexer);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::vector<std::unique_ptr<Expression>> expressions_; ///< 模板表达式列表
};

} // namespace compiler
} // namespace mjs
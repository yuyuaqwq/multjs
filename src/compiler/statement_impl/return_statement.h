/**
 * @file return_statement.h
 * @brief return语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"

#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class ReturnStatement
 * @brief return语句
 */
class ReturnStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param argument 返回值表达式（可选）
     */
    explicit ReturnStatement(SourceBytePosition start, SourceBytePosition end,
        std::unique_ptr<Expression> argument)
        : Statement(start, end), argument_(std::move(argument)) {}

    StatementType type() const noexcept override { return StatementType::kReturn; }

    /**
     * @brief 获取返回值表达式
     * @return 返回值表达式引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }

    /**
     * @brief 解析return语句
     * @param lexer 词法分析器
     * @return 解析后的return语句
     */
    static std::unique_ptr<ReturnStatement> ParseReturnStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const;

private:
    std::unique_ptr<Expression> argument_;
};

} // namespace compiler
} // namespace mjs
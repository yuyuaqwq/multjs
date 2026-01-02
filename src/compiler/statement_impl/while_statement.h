/**
 * @file while_statement.h
 * @brief while循环语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"
#include "block_statement.h"
#include "../expression.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class WhileStatement
 * @brief while循环语句
 */
class WhileStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param test 条件表达式
     * @param body 循环体
     */
    WhileStatement(SourceBytePosition start, SourceBytePosition end,
        std::unique_ptr<Expression> test,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        test_(std::move(test)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kWhile; }

    /**
     * @brief 获取条件表达式
     * @return 条件表达式引用
     */
    const std::unique_ptr<Expression>& test() const { return test_; }

    /**
     * @brief 获取循环体
     * @return 循环体引用
     */
    const std::unique_ptr<BlockStatement>& body() const { return body_; }

    /**
     * @brief 解析while语句
     * @param lexer 词法分析器
     * @return 解析后的while语句
     */
    static std::unique_ptr<WhileStatement> ParseWhileStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> test_;
    std::unique_ptr<BlockStatement> body_;
};

} // namespace compiler
} // namespace mjs
/**
 * @file if_statement.h
 * @brief if语句定义
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
 * @class IfStatement
 * @brief if条件语句
 */
class IfStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param test 条件表达式
     * @param consequent 条件为真时执行的语句块
     * @param alternate 条件为假时执行的语句（else分支）
     */
    IfStatement(SourcePosition start, SourcePosition end,
        std::unique_ptr<Expression> test,
        std::unique_ptr<BlockStatement> consequent,
        std::unique_ptr<Statement> alternate)
        : Statement(start, end),
        test_(std::move(test)),
        consequent_(std::move(consequent)),
        alternate_(std::move(alternate)) {}

    StatementType type() const noexcept override { return StatementType::kIf; }

    /**
     * @brief 获取条件表达式
     * @return 条件表达式引用
     */
    const std::unique_ptr<Expression>& test() const { return test_; }

    /**
     * @brief 获取条件为真时执行的语句块
     * @return 语句块引用
     */
    const std::unique_ptr<BlockStatement>& consequent() const { return consequent_; }

    /**
     * @brief 获取条件为假时执行的语句
     * @return 语句引用
     */
    const std::unique_ptr<Statement>& alternate() const { return alternate_; }

    /**
     * @brief 解析if语句
     * @param lexer 词法分析器
     * @return 解析后的if语句
     */
    static std::unique_ptr<IfStatement> ParseIfStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> test_;
    std::unique_ptr<BlockStatement> consequent_;
    std::unique_ptr<Statement> alternate_;
};

} // namespace compiler
} // namespace mjs
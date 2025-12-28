/**
 * @file for_statement.h
 * @brief for循环语句定义
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
 * @class ForStatement
 * @brief for循环语句
 */
class ForStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param init 初始化语句
     * @param test 条件表达式
     * @param update 更新表达式
     * @param body 循环体
     */
    ForStatement(SourcePosition start, SourcePosition end,
        std::unique_ptr<Statement> init,
        std::unique_ptr<Expression> test,
        std::unique_ptr<Expression> update,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        init_(std::move(init)),
        test_(std::move(test)),
        update_(std::move(update)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kFor; }

    /**
     * @brief 获取初始化语句
     * @return 初始化语句引用
     */
    const std::unique_ptr<Statement>& init() const { return init_; }

    /**
     * @brief 获取条件表达式
     * @return 条件表达式引用
     */
    const std::unique_ptr<Expression>& test() const { return test_; }

    /**
     * @brief 获取更新表达式
     * @return 更新表达式引用
     */
    const std::unique_ptr<Expression>& update() const { return update_; }

    /**
     * @brief 获取循环体
     * @return 循环体引用
     */
    const std::unique_ptr<BlockStatement>& body() const { return body_; }

    /**
     * @brief 解析for语句
     * @param lexer 词法分析器
     * @return 解析后的for语句
     */
    static std::unique_ptr<ForStatement> ParseForStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Statement> init_;
    std::unique_ptr<Expression> test_;
    std::unique_ptr<Expression> update_;
    std::unique_ptr<BlockStatement> body_;
};

} // namespace compiler
} // namespace mjs
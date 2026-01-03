/**
 * @file try_statement.h
 * @brief try语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/statement_impl/catch_clause.h"
#include "src/compiler/statement_impl/finally_clause.h"

namespace mjs {
namespace compiler {

/**
 * @class TryStatement
 * @brief try语句
 */
class TryStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param block try块
     * @param handler catch子句（可选）
     * @param finalizer finally子句（可选）
     */
    TryStatement(SourceBytePosition start, SourceBytePosition end,
        std::unique_ptr<BlockStatement> block,
        std::unique_ptr<CatchClause> handler,
        std::unique_ptr<FinallyClause> finalizer)
        : Statement(start, end),
        block_(std::move(block)),
        handler_(std::move(handler)),
        finalizer_(std::move(finalizer)) {}

    StatementType type() const noexcept override { return StatementType::kTry; }

    /**
     * @brief 获取try块
     * @return try块引用
     */
    const std::unique_ptr<BlockStatement>& block() const { return block_; }

    /**
     * @brief 获取catch子句
     * @return catch子句引用
     */
    const std::unique_ptr<CatchClause>& handler() const { return handler_; }

    /**
     * @brief 获取finally子句
     * @return finally子句引用
     */
    const std::unique_ptr<FinallyClause>& finalizer() const { return finalizer_; }

    /**
     * @brief 解析try语句
     * @param lexer 词法分析器
     * @return 解析后的try语句
     */
    static std::unique_ptr<TryStatement> ParseTryStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<BlockStatement> block_;
    std::unique_ptr<CatchClause> handler_;
    std::unique_ptr<FinallyClause> finalizer_;
};

} // namespace compiler
} // namespace mjs
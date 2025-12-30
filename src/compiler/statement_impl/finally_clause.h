/**
 * @file finally_clause.h
 * @brief finally子句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"
#include "block_statement.h"

namespace mjs {
namespace compiler {

/**
 * @class FinallyClause
 * @brief finally子句
 */
class FinallyClause : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param body finally块
     */
    explicit FinallyClause(SourcePosition start, SourcePosition end,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end), body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kFinally; }

    /**
     * @brief 获取finally块
     * @return finally块引用
     */
    const std::unique_ptr<BlockStatement>& body() const { return body_; }

    /**
     * @brief 解析finally子句
     * @param lexer 词法分析器
     * @return 解析后的finally子句
     */
    static std::unique_ptr<FinallyClause> ParseFinallyClause(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<BlockStatement> body_;
};

} // namespace compiler
} // namespace mjs
/**
 * @file catch_clause.h
 * @brief catch子句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"
#include "block_statement.h"
#include "../expression_impl/identifier.h"

namespace mjs {
namespace compiler {

/**
 * @class CatchClause
 * @brief catch子句
 */
class CatchClause : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param param 异常参数
     * @param body catch块
     */
    CatchClause(SourcePosition start, SourcePosition end,
        std::unique_ptr<Identifier> param,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        param_(std::move(param)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kCatch; }

    /**
     * @brief 获取异常参数
     * @return 异常参数引用
     */
    const std::unique_ptr<Identifier>& param() const { return param_; }

    /**
     * @brief 获取catch块
     * @return catch块引用
     */
    const std::unique_ptr<BlockStatement>& body() const { return body_; }

    /**
     * @brief 解析catch子句
     * @param lexer 词法分析器
     * @return 解析后的catch子句
     */
    static std::unique_ptr<CatchClause> ParseCatchClause(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Identifier> param_;
    std::unique_ptr<BlockStatement> body_;
};

} // namespace compiler
} // namespace mjs
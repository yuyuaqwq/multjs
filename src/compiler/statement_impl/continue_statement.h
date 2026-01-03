/**
 * @file continue_statement.h
 * @brief continue语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include <optional>

#include "src/compiler/statement.h"
#include "src/compiler/lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class ContinueStatement
 * @brief continue语句
 */
class ContinueStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param label 标签名（可选）
     */
    explicit ContinueStatement(SourceBytePosition start, SourceBytePosition end,
        std::optional<std::string> label)
        : Statement(start, end), label_(std::move(label)) {}

    StatementType type() const noexcept override { return StatementType::kContinue; }

    /**
     * @brief 获取标签名
     * @return 标签名引用
     */
    const std::optional<std::string>& label() const { return label_; }

    /**
     * @brief 解析continue语句
     * @param lexer 词法分析器
     * @return 解析后的continue语句
     */
    static std::unique_ptr<ContinueStatement> ParseContinueStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::optional<std::string> label_;
};

} // namespace compiler
} // namespace mjs
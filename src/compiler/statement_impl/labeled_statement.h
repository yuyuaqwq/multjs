/**
 * @file labeled_statement.h
 * @brief 标签语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>

#include "src/compiler/statement.h"

namespace mjs {
namespace compiler {

/**
 * @class LabeledStatement
 * @brief 标签语句
 */
class LabeledStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param label 标签名
     * @param body 语句体
     */
    LabeledStatement(SourceBytePosition start, SourceBytePosition end,
        std::string label,
        std::unique_ptr<Statement> body)
        : Statement(start, end),
        label_(std::move(label)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kLabeled; }

    /**
     * @brief 获取标签名
     * @return 标签名引用
     */
    const std::string& label() const { return label_; }

    /**
     * @brief 获取语句体
     * @return 语句体引用
     */
    const std::unique_ptr<Statement>& body() const { return body_; }

    /**
     * @brief 解析标签语句
     * @param lexer 词法分析器
     * @return 解析后的标签语句
     */
    static std::unique_ptr<LabeledStatement> ParseLabeledStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const;

private:
    std::string label_;
    std::unique_ptr<Statement> body_;
};

} // namespace compiler
} // namespace mjs
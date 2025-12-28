/**
 * @file break_statement.h
 * @brief break语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include <optional>

#include "../statement.h"

namespace mjs {
namespace compiler {

/**
 * @class BreakStatement
 * @brief break语句
 */
class BreakStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param label 标签名（可选）
     */
    explicit BreakStatement(SourcePosition start, SourcePosition end,
        std::optional<std::string> label)
        : Statement(start, end), label_(std::move(label)) {}

    StatementType type() const noexcept override { return StatementType::kBreak; }

    /**
     * @brief 获取标签名
     * @return 标签名引用
     */
    const std::optional<std::string>& label() const { return label_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析break语句
     * @param lexer 词法分析器
     * @return 解析后的break语句
     */
    static std::unique_ptr<BreakStatement> ParseBreakStatement(Lexer* lexer);

private:
    std::optional<std::string> label_;
};

} // namespace compiler
} // namespace mjs
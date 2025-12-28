/**
 * @file block_statement.h
 * @brief 块语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>

#include "../statement.h"
#include "../expression.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class BlockStatement
 * @brief 块语句，包含多个语句
 */
class BlockStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param statements 语句列表
     */
    BlockStatement(SourcePosition start, SourcePosition end, std::vector<std::unique_ptr<Statement>>&& statements)
        : Statement(start, end), statements_(std::move(statements)) {}

    StatementType type() const noexcept override { return StatementType::kBlock; }

    /**
     * @brief 获取语句列表
     * @return 语句列表引用
     */
    const std::vector<std::unique_ptr<Statement>>& statements() const { return statements_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析块语句
     * @param lexer 词法分析器
     * @return 解析后的块语句
     */
    static std::unique_ptr<BlockStatement> ParseBlockStatement(Lexer* lexer);

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

} // namespace compiler
} // namespace mjs
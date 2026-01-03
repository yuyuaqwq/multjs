/**
 * @file throw_statement.h
 * @brief throw语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "src/compiler/statement.h"
#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @class ThrowStatement
 * @brief throw语句
 */
class ThrowStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param argument 异常表达式
     */
    explicit ThrowStatement(SourceBytePosition start, SourceBytePosition end,
                          std::unique_ptr<Expression> argument)
        : Statement(start, end), argument_(std::move(argument)) {}

    StatementType type() const noexcept override { return StatementType::kThrow; }

    /**
     * @brief 获取异常表达式
     * @return 异常表达式引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }

    /**
     * @brief 解析throw语句
     * @param lexer 词法分析器
     * @return 解析后的throw语句
     */
    static std::unique_ptr<ThrowStatement> ParseThrowStatement(Lexer* lexer);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> argument_;
};

} // namespace compiler
} // namespace mjs
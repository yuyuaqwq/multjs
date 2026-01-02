/**
 * @file expression_statement.h
 * @brief 表达式语句定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class ExpressionStatement
 * @brief 表达式语句
 */
class ExpressionStatement : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param expression 表达式
     */
    ExpressionStatement(SourceBytePosition start, SourceBytePosition end, std::unique_ptr<Expression> expression)
        : Statement(start, end), expression_(std::move(expression)) {}

    StatementType type() const noexcept override { return StatementType::kExpression; }

    /**
     * @brief 获取表达式
     * @return 表达式引用
     */
    const std::unique_ptr<Expression>& expression() const { return expression_; }

	/**
	 * @brief 解析表达式语句
	 * @return 解析后的表达式语句
	 */
	static std::unique_ptr<ExpressionStatement> ParseExpressionStatement(Lexer* lexer);

    virtual void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> expression_;
};

} // namespace compiler
} // namespace mjs
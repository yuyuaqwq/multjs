/**
 * @file class_declaration.h
 * @brief 类声明
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "src/compiler/statement.h"
#include "src/compiler/expression_impl/class_expression.h"

namespace mjs {
namespace compiler {

class Expression;

/**
 * @class ClassDeclaration
 * @brief 类声明语句
 */
class ClassDeclaration : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 语句起始位置
     * @param end 语句结束位置
     * @param id 类标识符
     * @param super_class 父类表达式（可选）
     * @param elements 类元素列表
     */
    ClassDeclaration(
        SourceBytePosition start,
        SourceBytePosition end,
        std::unique_ptr<ClassExpression> expression
        )
        : Statement(start, end)
        , expression_(std::move(expression)) {}

    /**
     * @brief 获取语句类型
     * @return 语句类型
     */
    StatementType type() const noexcept override { return StatementType::kClassDeclaration; }

    /**
     * @brief 获取类标识符
     * @return 类标识符
     */
    const std::string& id() const { return expression_->id().value(); }

    /**
     * @brief 获取父类表达式
     * @return 父类表达式
     */
    const std::unique_ptr<Expression>& super_class() const { return expression_->super_class(); }

    /**
     * @brief 获取类元素列表
     * @return 类元素列表
     */
    const std::vector<ClassElement>& elements() const { return expression_->elements(); }

    /**
     * @brief 判断是否有父类
     * @return 是否有父类
     */
    bool has_super_class() const { return expression_->has_super_class(); }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析类声明
     * @param lexer 词法分析器
     * @return 解析后的类声明
     */
    static std::unique_ptr<ClassDeclaration> ParseClassDeclaration(Lexer* lexer);

private:
    std::unique_ptr<ClassExpression> expression_;
};

} // namespace compiler
} // namespace mjs

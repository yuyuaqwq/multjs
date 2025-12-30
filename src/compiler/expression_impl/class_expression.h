/**
 * @file class_expression.h
 * @brief 类表达式
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "../expression.h"
#include "class_element.h"

namespace mjs {
namespace compiler {

/**
 * @class ClassExpression
 * @brief 类表达式
 */
class ClassExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param id 类标识符（可选）
     * @param super_class 父类表达式（可选）
     * @param elements 类元素列表
     */
    ClassExpression(
        SourcePosition start,
        SourcePosition end,
        std::optional<std::string> id,
        std::unique_ptr<Expression> super_class,
        std::vector<ClassElement>&& elements)
        : Expression(start, end)
        , id_(std::move(id))
        , super_class_(std::move(super_class))
        , elements_(std::move(elements)) {}

    /**
     * @brief 获取类标识符
     * @return 类标识符
     */
    const std::optional<std::string>& id() const { return id_; }

    /**
     * @brief 获取父类表达式
     * @return 父类表达式
     */
    const std::unique_ptr<Expression>& super_class() const { return super_class_; }

    /**
     * @brief 获取类元素列表
     * @return 类元素列表
     */
    const std::vector<ClassElement>& elements() const { return elements_; }

    /**
     * @brief 判断是否有父类
     * @return 是否有父类
     */
    bool has_super_class() const { return super_class_ != nullptr; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析类表达式
     * @param lexer 词法分析器
     * @return 解析后的类表达式
     */
    static std::unique_ptr<ClassExpression> ParseClassExpression(Lexer* lexer, bool force_parse_class_name);

private:
    std::optional<std::string> id_;               ///< 类标识符
    std::unique_ptr<Expression> super_class_;     ///< 父类表达式
    std::vector<ClassElement> elements_;          ///< 类元素列表
};

} // namespace compiler
} // namespace mjs

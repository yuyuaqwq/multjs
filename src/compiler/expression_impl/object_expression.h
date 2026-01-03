/**
 * @file object_expression.h
 * @brief 对象表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @class ObjectExpression
 * @brief 对象表达式
 */
class ObjectExpression : public Expression {
public:
    /**
     * @struct Property
     * @brief 对象属性定义
     */
    struct Property {
        std::string key;                    ///< 属性键名
        std::unique_ptr<Expression> value; ///< 属性值表达式
        bool shorthand;                    ///< 是否为简写属性
        bool computed;                     ///< 是否为计算属性
    };

    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param properties 对象属性列表
     */
    ObjectExpression(SourceBytePosition start, SourceBytePosition end, std::vector<Property>&& properties)
        : Expression(start, end), properties_(std::move(properties)) {}

    /**
     * @brief 获取对象属性列表
     * @return 属性列表的常量引用
     */
    const std::vector<Property>& properties() const { return properties_; }

    static std::unique_ptr<ObjectExpression> ParseObjectExpression(Lexer* lexer);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::vector<Property> properties_; ///< 对象属性列表
};

} // namespace compiler
} // namespace mjs
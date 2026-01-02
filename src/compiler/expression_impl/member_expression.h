/**
 * @file member_expression.h
 * @brief 成员访问表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class MemberExpression
 * @brief 成员访问表达式
 */
class MemberExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param object 对象表达式
     * @param property 属性表达式
     * @param is_method_call 是否为方法调用
     * @param computed 是否为计算属性
     * @param optional 是否为可选链访问
     */
    MemberExpression(SourceBytePosition start, SourceBytePosition end,
                std::unique_ptr<Expression> object,
                std::unique_ptr<Expression> property,
                bool is_method_call, bool computed, bool optional)
        : Expression(start, end), object_(std::move(object)),
        property_(std::move(property)), computed_(computed),
        is_method_call_(is_method_call), optional_(optional) {
        set_value_category(ValueCategory::kLValue);
    }


    /**
     * @brief 获取对象表达式
     * @return 对象表达式的常量引用
     */
    const std::unique_ptr<Expression>& object() const { return object_; }

    /**
     * @brief 获取属性表达式
     * @return 属性表达式的常量引用
     */
    const std::unique_ptr<Expression>& property() const { return property_; }

    /**
     * @brief 判断是否为方法调用
     * @return 如果是方法调用则返回true，否则返回false
     */
    bool is_method_call() const { return is_method_call_; }

    /**
     * @brief 判断是否为计算属性
     * @return 如果是计算属性则返回true，否则返回false
     */
    bool computed() const { return computed_; }

    /**
     * @brief 判断是否为可选链访问
     * @return 如果是可选链访问则返回true，否则返回false
     */
    bool optional() const { return optional_; }

    static std::unique_ptr<MemberExpression> ParseMemberExpression(Lexer* lexer, std::unique_ptr<Expression> object);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> object_;   ///< 对象表达式
    std::unique_ptr<Expression> property_; ///< 属性表达式
    bool computed_;                        ///< 是否为计算属性
    bool is_method_call_;                  ///< 是否为方法调用
    bool optional_;                        ///< 是否为可选链访问
};

} // namespace compiler
} // namespace mjs
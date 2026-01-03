/**
 * @file class_element.h
 * @brief 类元素定义
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>
#include <optional>

#include "src/compiler/expression.h"

namespace mjs {
namespace compiler {

/**
 * @enum MethodKind
 * @brief 方法类型
 */
enum class MethodKind {
    kConstructor,    ///< 构造函数
    kMethod,         ///< 普通方法
    kGetter,         ///< getter
    kSetter,         ///< setter
    kStatic,         ///< 静态方法
    kStaticGetter,   ///< 静态getter
    kStaticSetter,   ///< 静态setter
    kField,          ///< 类字段
    kStaticField,    ///< 静态类字段
};

/**
 * @class ClassElement
 * @brief 类元素定义（方法、字段等）
 */
class ClassElement {
public:
    /**
     * @brief 构造函数
     */
    ClassElement(
        MethodKind kind,
        std::string key,
        std::unique_ptr<Expression> value,
        bool computed = false)
        : kind_(kind)
        , key_(std::move(key))
        , value_(std::move(value))
        , computed_(computed) {}

    /**
     * @brief 获取方法类型
     * @return 方法类型
     */
    MethodKind kind() const { return kind_; }

    /**
     * @brief 获取键名
     * @return 键名
     */
    const std::string& key() const { return key_; }

    /**
     * @brief 获取值表达式
     * @return 值表达式
     */
    const std::unique_ptr<Expression>& value() const { return value_; }

    /**
     * @brief 判断是否为计算属性
     * @return 是否为计算属性
     */
    bool is_computed() const { return computed_; }

    /**
     * @brief 判断是否为静态成员
     * @return 是否为静态成员
     */
    bool is_static() const {
        return kind_ == MethodKind::kStatic ||
               kind_ == MethodKind::kStaticGetter ||
               kind_ == MethodKind::kStaticSetter ||
               kind_ == MethodKind::kStaticField;
    }

private:
    MethodKind kind_;                        ///< 方法类型
    std::string key_;                        ///< 键名
    std::unique_ptr<Expression> value_;      ///< 值表达式（通常是FunctionExpression）
    bool computed_;                          ///< 是否为计算属性名
};

} // namespace compiler
} // namespace mjs

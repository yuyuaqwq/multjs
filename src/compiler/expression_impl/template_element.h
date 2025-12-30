/**
 * @file template_element.h
 * @brief 模板字符串元素表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class TemplateElement
 * @brief 模板字符串元素表达式
 */
class TemplateElement : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param value 模板元素值
     */
    TemplateElement(SourcePosition start, SourcePosition end, std::string&& value)
        : Expression(start, end), value_(std::move(value)) {}


    /**
     * @brief 获取模板元素值
     * @return 模板元素值的常量引用
     */
    const std::string& value() const { return value_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string value_; ///< 模板元素值
};

} // namespace compiler
} // namespace mjs
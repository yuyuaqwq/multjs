/**
 * @file identifier.h
 * @brief 标识符表达式
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
 * @class Identifier
 * @brief 标识符表达式
 */
class Identifier : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param name 标识符名称
     */
    Identifier(SourcePosition start, SourcePosition end, std::string&& name)
        : Expression(start, end), name_(std::move(name)) {
        set_value_category(ValueCategory::kLValue);
    }


    /**
     * @brief 获取标识符名称
     * @return 标识符名称的常量引用
     */
    const std::string& name() const { return name_; }

    static std::unique_ptr<Identifier> ParseIdentifier(Lexer* lexer);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string name_; ///< 标识符名称
};

} // namespace compiler
} // namespace mjs
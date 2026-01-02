/**
 * @file type_annotation.h
 * @brief 类型注解定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "type_base.h"

#include "../statement.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class TypeAnnotation
 * @brief 类型注解
 */
class TypeAnnotation : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param type_p 类型
     */
    TypeAnnotation(SourceBytePosition start, SourceBytePosition end, std::unique_ptr<Type>&& type_p)
        : Statement(start, end), type_p_(std::move(type_p)) {}

    StatementType type() const noexcept override { return StatementType::kTypeAnnotation; }

    /**
     * @brief 获取类型
     * @return 类型引用
     */
    const std::unique_ptr<Type>& type_p() const { return type_p_; }

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
    * @brief 尝试解析类型注解
    * @param lexer 词法分析器
    * @return 解析后的类型注解
    */
    static std::unique_ptr<TypeAnnotation> TryParseTypeAnnotation(Lexer* lexer);

private:
    std::unique_ptr<Type> type_p_;
};


} // namespace compiler
} // namespace mjs
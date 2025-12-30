/**
 * @file union_type.h
 * @brief 联合类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>

#include "type_base.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class UnionType
 * @brief 联合类型
 */
class UnionType : public Type {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param types 类型列表
     */
    UnionType(SourcePosition start, SourcePosition end, std::vector<std::unique_ptr<Type>>&& types)
        : Type(start, end), types_(std::move(types)) {}

    StatementType type() const noexcept override { return StatementType::kUnionType; }

    /**
     * @brief 获取类型列表
     * @return 类型列表引用
     */
    const std::vector<std::unique_ptr<Type>>& types() const { return types_; }

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override {}

private:
    std::vector<std::unique_ptr<Type>> types_;
};

/**
 * @brief 解析联合类型
 * @param lexer 词法分析器
 * @return 解析后的联合类型
 */
std::unique_ptr<UnionType> ParseUnionType(Lexer* lexer);

} // namespace compiler
} // namespace mjs
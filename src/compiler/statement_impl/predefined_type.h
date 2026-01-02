/**
 * @file predefined_type.h
 * @brief 预定义类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "type_base.h"

namespace mjs {
namespace compiler {

/**
 * @class PredefinedType
 * @brief 预定义类型
 */
class PredefinedType : public Type {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param keyword 类型关键字
     */
    PredefinedType(SourceBytePosition start, SourceBytePosition end, PredefinedTypeKeyword keyword)
        : Type(start, end), keyword_(keyword) {}

    StatementType type() const noexcept override { return StatementType::kPredefinedType; }

    /**
     * @brief 获取类型关键字
     * @return 类型关键字引用
     */
    const PredefinedTypeKeyword& keyword() const { return keyword_; }

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override {}

private:
    PredefinedTypeKeyword keyword_;
};

} // namespace compiler
} // namespace mjs
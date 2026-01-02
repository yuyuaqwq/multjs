/**
 * @file literal_type.h
 * @brief 字面量类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "type_base.h"
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class LiteralType
 * @brief 字面量类型
 */
class LiteralType : public Type {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param value 字面量值
     */
    LiteralType(SourceBytePosition start, SourceBytePosition end, std::unique_ptr<Expression>&& value)
        : Type(start, end), value_(std::move(value)) {}

    StatementType type() const noexcept override { return StatementType::kLiteralType; }

    /**
     * @brief 获取字面量值
     * @return 字面量值引用
     */
    const std::unique_ptr<Expression>& value() const { return value_; }

private:
    std::unique_ptr<Expression> value_;
};

} // namespace compiler
} // namespace mjs
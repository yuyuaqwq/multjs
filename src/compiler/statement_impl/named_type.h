/**
 * @file named_type.h
 * @brief 命名类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>

#include "type_base.h"

namespace mjs {
namespace compiler {

/**
 * @class NamedType
 * @brief 命名类型
 */
class NamedType : public Type {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param name 类型名
     */
    NamedType(SourceBytePosition start, SourceBytePosition end, std::string&& name)
        : Type(start, end), name_(std::move(name)) {}

    StatementType type() const noexcept override { return StatementType::kNamedType; }

    /**
     * @brief 获取类型名
     * @return 类型名引用
     */
    const std::string& name() const { return name_; }

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override {}

private:
    std::string name_;
};

} // namespace compiler
} // namespace mjs
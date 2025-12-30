/**
 * @file function_type.h
 * @brief 函数类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>

#include "type_base.h"

namespace mjs {
namespace compiler {

/**
 * @class FunctionType
 * @brief 函数类型
 */
class FunctionType : public Type {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param return_types 返回类型
     * @param param_types 参数类型列表
     */
    FunctionType(SourcePosition start, SourcePosition end,
        std::unique_ptr<Type> return_types,
        std::vector<std::unique_ptr<Type>> param_types)
        : Type(start, end),
        return_types_(std::move(return_types)),
        param_types_(std::move(param_types)) {}

    StatementType type() const noexcept override { return StatementType::kFunctionType; }

    /**
     * @brief 获取返回类型
     * @return 返回类型引用
     */
    const std::unique_ptr<Type>& return_types() const { return return_types_; }

    /**
     * @brief 获取参数类型列表
     * @return 参数类型列表引用
     */
    const std::vector<std::unique_ptr<Type>>& param_types() const { return param_types_; }

private:
    std::unique_ptr<Type> return_types_;
    std::vector<std::unique_ptr<Type>> param_types_;
};

} // namespace compiler
} // namespace mjs
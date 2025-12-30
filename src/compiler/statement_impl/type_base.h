/**
 * @file type_base.h
 * @brief 类型基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include "../statement.h"

namespace mjs {
namespace compiler {

/**
 * @enum PredefinedTypeKeyword
 * @brief 预定义类型关键字
 */
enum class PredefinedTypeKeyword {
    kNumber,
    kString,
    kBoolean,
    kAny,
    kVoid,
};

/**
 * @class Type
 * @brief 类型基类
 */
class Type : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     */
    Type(SourcePosition start, SourcePosition end)
        : Statement(start, end) {}
};

} // namespace compiler
} // namespace mjs
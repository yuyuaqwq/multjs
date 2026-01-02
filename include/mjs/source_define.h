/**
 * @file source_define.h
 * @brief JavaScript 源代码位置类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的源代码位置类型系统，包括源代码位置、
 * 行号、列号等类型定义，用于源代码位置处理和调试信息管理。
 */

#pragma once

#include <cstdint>
#include <string>

namespace mjs {

using Source = std::string; ///< 源代码类型
using SourceBytePosition = uint32_t;    ///< 源代码位置类型
using SourceLine = uint32_t;   ///< 源代码行号类型
using SourceColumn = uint32_t; ///< 源代码列号类型

constexpr SourceBytePosition kSourceBytePositionInvalid = 0xffffffff; ///< 无效源代码位置常量
constexpr SourceLine kSourceLineInvalid = 0;        ///< 无效源代码行号常量

} // namespace mjs
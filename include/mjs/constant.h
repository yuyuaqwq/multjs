/**
 * @file constant.h
 * @brief JavaScript 常量索引系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的常量索引系统，包括常量索引类型
 * 和相关的常量定义，用于区分全局常量池和本地常量池的索引。
 */

#pragma once

#include <cstdint>
#include <cassert>

namespace mjs {

/**
 * @brief 常量索引类型
 *
 * 用于标识常量在常量池中的位置：
 * - 0 表示无效索引
 * - 正数，从1开始，表示全局常量池索引
 * - 负数，从-1开始，表示本地常量池索引
 */
using ConstIndex = int32_t;

/**
 * @brief 无效常量索引常量
 */
constexpr ConstIndex kConstIndexInvalid = 0;

} // namespace mjs 
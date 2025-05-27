#pragma once

#include <cstdint>
#include <cassert>

namespace mjs {

// 0表示无效
// 正数，从1开始，表示全局常量池索引
// 负数，从-1开始，表示局部常量池索引

using ConstIndex = int32_t;
constexpr ConstIndex kConstIndexInvalid = 0;

} // namespace mjs 
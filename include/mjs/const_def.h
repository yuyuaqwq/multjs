#pragma once

#include <cstdint>

namespace mjs {

// 正数，从0开始，表示全局常量池索引
// 负数，从-1开始，表示局部常量池索引
using ConstIndex = int32_t;

inline bool IsGlobalConstIndex(ConstIndex idx) {
    return idx >= 0;
}

inline bool IsLocalConstIndex(ConstIndex idx) {
    return idx < 0;
}

} // namespace mjs 
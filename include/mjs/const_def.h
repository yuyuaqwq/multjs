#pragma once

#include <cstdint>

namespace mjs {

// 0表示无效
// 正数，从1开始，表示全局常量池索引
// 负数，从-1开始，表示局部常量池索引
using ConstIndex = int32_t;

constexpr ConstIndex kConstInvaildIndex = 0;

inline bool IsGlobalConstIndex(ConstIndex idx) {
    return idx > 0;
}

inline bool IsLocalConstIndex(ConstIndex idx) {
    return idx < 0;
}

inline ConstIndex ConstToGlobalIndex(ConstIndex idx) {
    return idx;
}

inline ConstIndex ConstToLocalIndex(ConstIndex idx) {
    return -idx;
}

inline ConstIndex LocalToConstIndex(ConstIndex idx) {
    return -idx;
}

inline ConstIndex GlobalToConstIndex(ConstIndex idx) {
    return idx;
}

} // namespace mjs 
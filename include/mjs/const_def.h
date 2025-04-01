#pragma once

#include <cstdint>

namespace mjs {

// 0��ʾ��Ч
// ��������1��ʼ����ʾȫ�ֳ���������
// ��������-1��ʼ����ʾ�ֲ�����������
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
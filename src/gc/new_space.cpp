/**
 * @file new_space.cpp
 * @brief 新生代内存空间实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <mjs/gc/new_space.h>

#include <cstring>
#include <algorithm>

namespace mjs {

// ==================== NewSpace Implementation ====================

NewSpace::NewSpace() = default;

NewSpace::~NewSpace() {
    if (from_space_) {
        delete[] from_space_;
    }
    if (to_space_) {
        delete[] to_space_;
    }
}

bool NewSpace::Initialize() {
    // 分配两个半区
    from_space_ = new uint8_t[kNewSpaceSemiSize];
    to_space_ = new uint8_t[kNewSpaceSemiSize];

    if (!from_space_ || !to_space_) {
        return false;
    }

    top_ = from_space_;
    to_space_top_ = to_space_;  // 初始化To空间分配指针
    return true;
}

void* NewSpace::Allocate(size_t* size) {
    // 对齐大小
    *size = AlignGCObjectSize(*size);

    if (!HasSpace(*size)) {
        return nullptr;
    }

    void* result = top_;
    top_ += *size;

    // 清零内存
    // std::memset(result, 0, size);
    return result;
}

void* NewSpace::AllocateInToSpace(size_t* size) {
    // 对齐大小
    *size = AlignGCObjectSize(*size);

    // 检查To空间是否有足够空间
    if (to_space_top_ + *size > to_space_ + kNewSpaceSemiSize) {
        return nullptr;
    }

    void* result = to_space_top_;
    to_space_top_ += *size;

    // 清零内存
    // std::memset(result, 0, size);
    return result;
}

bool NewSpace::HasSpace(size_t size) const {
    return top_ + AlignGCObjectSize(size) <= from_space_ + kNewSpaceSemiSize;
}

void NewSpace::SwapSpaces() {
    std::swap(from_space_, to_space_);
    std::swap(top_, to_space_top_);
}

void NewSpace::Reset(uint8_t* new_top) {
    top_ = new_top;
}

void NewSpace::IterateObjects(ObjectCallback callback, void* data) {
    uint8_t* current = from_space_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj, data);
        current += obj->header()->size();
    }
}

} // namespace mjs

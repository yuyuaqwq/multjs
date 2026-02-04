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
    if (eden_space_) {
        delete[] eden_space_;
    }
    if (survivor_from_) {
        delete[] survivor_from_;
    }
    if (survivor_to_) {
        delete[] survivor_to_;
    }
}

bool NewSpace::Initialize() {
    // 分配三个区域：Eden、Survivor From、Survivor To
    eden_space_ = new uint8_t[kEdenSpaceSize];
    survivor_from_ = new uint8_t[kSurvivorSpaceSize];
    survivor_to_ = new uint8_t[kSurvivorSpaceSize];

    if (!eden_space_ || !survivor_from_ || !survivor_to_) {
        return false;
    }

    eden_top_ = eden_space_;
    survivor_from_top_ = survivor_from_;
    survivor_to_top_ = survivo r_to_;

    return true;
}

void* NewSpace::Allocate(size_t* size) {
    // 对齐大小
    *size = AlignGCObjectSize(*size);

    if (!HasSpace(*size)) {
        return nullptr;
    }

    void* result = eden_top_;
    eden_top_ += *size;

    // 清零内存
    // std::memset(result, 0, size);
    return result;
}

void* NewSpace::AllocateInToSpace(size_t* size) {
    // 对齐大小
    *size = AlignGCObjectSize(*size);

    // 检查Survivor To空间是否有足够空间
    if (survivor_to_top_ + *size > survivor_to_ + kSurvivorSpaceSize) {
        return nullptr;
    }

    void* result = survivor_to_top_;
    survivor_to_top_ += *size;

    // 清零内存
    // std::memset(result, 0, size);
    return result;
}

bool NewSpace::HasSpace(size_t size) const {
    return eden_top_ + size <= eden_space_ + kEdenSpaceSize;
}

void NewSpace::SwapSurvivorSpaces() {
    std::swap(survivor_from_, survivor_to_);
    std::swap(survivor_from_top_, survivor_to_top_);
}

void NewSpace::IterateObjects(ObjectCallback callback, void* data) {
    // 遍历Eden区中的所有对象
    uint8_t* current = eden_space_;
    while (current < eden_top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj, data);
        current += obj->header()->size();
    }

    // 遍历Survivor From区中的所有对象
    current = survivor_from_;
    while (current < survivor_from_top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj, data);
        current += obj->header()->size();
    }
}

} // namespace mjs

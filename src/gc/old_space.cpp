/**
 * @file old_space.cpp
 * @brief 老年代内存空间实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <mjs/gc/old_space.h>

#include <cstring>

#include <mjs/value/value.h>
#include <mjs/value/object/object.h>

namespace mjs {

// ==================== OldSpace Implementation ====================

OldSpace::OldSpace() = default;

OldSpace::~OldSpace() {
    if (space_start_) {
        delete[] space_start_;
    }
}

bool OldSpace::Initialize(size_t initial_size) {
    space_start_ = new uint8_t[initial_size];
    if (!space_start_) {
        return false;
    }

    capacity_ = initial_size;
    top_ = space_start_;
    used_size_ = 0;
    return true;
}

void* OldSpace::Allocate(size_t* size) {
    *size = AlignGCObjectSize(*size);

    // 检查是否有足够空间
    if (top_ + *size > space_start_ + capacity_) {
        // 空间不足，返回 nullptr
        // 调用者应触发 GC 或扩容
        return nullptr;
    }

    void* result = top_;
    top_ += *size;
    used_size_ += *size;

    // 清零内存
    // std::memset(result, 0, size);
    return result;
}

bool OldSpace::Expand(size_t min_size) {
    // 计算新的容量（至少是当前的两倍，或足够容纳 min_size）
    size_t new_capacity = capacity_ * 2;
    if (new_capacity < capacity_ + min_size) {
        new_capacity = capacity_ + min_size + kOldSpaceInitialSize;
    }

    // 分配新的内存块
    uint8_t* new_space_start = new uint8_t[new_capacity];
    if (!new_space_start) {
        return false;
    }

    uint8_t* new_top = new_space_start;

    // 复制所有对象到新内存，并设置转发地址
    uint8_t* current = space_start_;
    while (current < top_) {
        GCObject* old_obj = reinterpret_cast<GCObject*>(current);
        size_t obj_size = old_obj->header()->size();

        // 复制对象到新内存
        GCObject* new_obj = reinterpret_cast<GCObject*>(new_top);
        std::memcpy(new_top, current, obj_size);

        // 在旧对象中设置转发地址
        old_obj->header()->SetForwardingAddress(new_obj);

        // 调用对象的移动回调
        new_obj->GCMoved(old_obj);

        new_top += obj_size;
        current += obj_size;
    }

    // 保存旧内存地址（用于后续释放）
    old_space_start_ = space_start_;
    old_top_ = top_;

    // 更新指针和容量
    space_start_ = new_space_start;
    top_ = new_top;
    capacity_ = new_capacity;

    return true;
}

void OldSpace::FinishExpand() {
    if (old_space_start_) {
        // 清除旧内存中的转发标记
        uint8_t* current = old_space_start_;
        while (current < old_top_) {
            GCObject* obj = reinterpret_cast<GCObject*>(current);
            obj->header()->SetForwardingAddress(nullptr);
            current += obj->header()->size();
        }

        // 释放旧内存
        delete[] old_space_start_;
        old_space_start_ = nullptr;
        old_top_ = nullptr;
    }
}

void OldSpace::IterateObjects(ObjectCallback callback, void* data) {
    uint8_t* current = space_start_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj, data);
        current += obj->header()->size();
    }
}

void OldSpace::IterateLiveObjects(ObjectCallback callback, void* data) {
    uint8_t* current = space_start_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        if (obj->header()->IsMarked()) {
            callback(obj, data);
        }
        current += obj->header()->size();
    }
}

void OldSpace::UpdateReference(Value* value) {
    if (!value || !value->IsObject()) {
        return;
    }

    Object* obj = &value->object();
    GCObject* gc_obj = static_cast<GCObject*>(obj);

    if (gc_obj->header()->generation() != GCGeneration::kOld) {
        return;
    }

    if (!gc_obj->header()->IsForwarded()) {
        return;
    }

    GCObject* new_obj = gc_obj->header()->GetForwardingAddress();
    if (new_obj != gc_obj) {
        *value = Value(static_cast<Object*>(new_obj));
    }
}

uint8_t* OldSpace::ComputeCompactTop() {
    uint8_t* new_top = space_start_;
    uint8_t* current = space_start_;

    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        if (obj->header()->IsMarked()) {
            new_top += obj->header()->size();
        }
        current += obj->header()->size();
    }

    return new_top;
}

void OldSpace::MoveObject(GCObject* obj, void* data) {
    MoveObjectData* move_data = static_cast<MoveObjectData*>(data);
    GCObject* new_addr = obj->header()->GetForwardingAddress();
    if (new_addr != obj) {
        size_t size = obj->header()->size();
        std::memmove(new_addr, obj, size);

        // 调用移动回调
        new_addr->GCMoved(obj);
    }
}

void OldSpace::ComputeForwardingAddr(GCObject* obj, void* data) {
    CompactForwardData* fwd_data = static_cast<CompactForwardData*>(data);
    if (obj->header()->IsMarked()) {
        // 使用内联转发指针存储新地址
        obj->header()->SetForwardingAddress(reinterpret_cast<GCObject*>(fwd_data->new_pos));
        fwd_data->new_pos += obj->header()->size();
    }
}

} // namespace mjs

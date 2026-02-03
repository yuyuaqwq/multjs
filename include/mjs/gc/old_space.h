/**
 * @file old_space.h
 * @brief 老年代内存空间定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了老年代内存空间（OldSpace），使用标记-压缩算法进行GC。
 */

#pragma once

#include <mjs/noncopyable.h>
#include <mjs/gc/gc_object.h>

namespace mjs {

/**
 * @brief 老年代初始大小（1MB）
 */
constexpr size_t kOldSpaceInitialSize = 1024 * 1024;

/**
 * @class OldSpace
 * @brief 老年代内存空间（使用标记-压缩算法）
 *
 * 老年代使用标记-压缩算法进行GC，减少内存碎片。
 *
 * @note 老年代不支持动态扩容，因为标记-压缩算法假设内存是连续的，
 *       扩容会导致对象移动和指针失效。如果空间不足，应触发 GC 或
 *       在初始化时指定更大的 initial_size。
 */
class OldSpace : public noncopyable {
public:
    /**
     * @brief 构造函数
     */
    OldSpace();

    /**
     * @brief 析构函数
     */
    ~OldSpace();

    /**
     * @brief 初始化老年代空间
     * @param initial_size 初始大小（也是最大大小，不支持扩容）
     * @return 是否初始化成功
     */
    bool Initialize(size_t initial_size = kOldSpaceInitialSize);

    /**
     * @brief 分配内存
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* Allocate(size_t* size);

    /**
     * @brief 扩展老年代空间（会移动所有对象）
     * @param min_size 最小需要的额外空间
     * @return 是否扩容成功
     * @note 扩容后会设置转发地址，调用者需要：
     *       1. 遍历所有根和对象，调用UpdateRefs更新引用
     *       2. 调用FinishExpand清除转发标记并释放旧内存
     */
    bool Expand(size_t min_size);

    /**
     * @brief 完成扩容（清除转发标记并释放旧内存）
     */
    void FinishExpand();

    /**
     * @brief 获取旧内存地址（扩容期间使用）
     */
    uint8_t* old_space_start() const { return old_space_start_; }

    /**
     * @brief 对象回调函数类型
     * @param obj 对象指针
     * @param data 用户数据
     */
    using ObjectCallback = void (*)(GCObject* obj, void* data);

    /**
     * @brief 遍历所有对象
     * @param callback 回调函数
     * @param data 用户数据
     */
    void IterateObjects(ObjectCallback callback, void* data);

    /**
     * @brief 遍历所有存活对象（已标记）
     * @param callback 回调函数
     * @param data 用户数据
     */
    void IterateLiveObjects(ObjectCallback callback, void* data);

    /**
     * @brief 获取已使用内存大小
     */
    size_t used_size() const { return used_size_; }

    /**
     * @brief 获取总容量
     */
    size_t capacity() const { return capacity_; }

    /**
     * @brief 获取空间起始地址
     */
    uint8_t* space_start() const { return space_start_; }

    /**
     * @brief 获取空间结束地址
     */
    uint8_t* space_end() const { return space_start_ + capacity_; }

    /**
     * @brief 获取当前分配位置
     */
    uint8_t* top() const { return top_; }

    /**
     * @brief 设置新的分配位置（压缩后使用）
     * @param new_top 新的top位置
     */
    void set_top(uint8_t* new_top) { top_ = new_top; }

    /**
     * @brief 计算压缩后的新位置
     * @return 新的top位置
     */
    uint8_t* ComputeCompactTop();

    /**
     * @brief 更新引用
     * @param value 值指针
     */
    static void UpdateReference(Value* value);

    // 移动对象
    struct MoveObjectData {
        GCHeap* heap;
    };

    static void MoveObject(GCObject* obj, void* data);

    // 计算转发地址
    struct CompactForwardData {
        uint8_t* new_pos;
    };

    static void ComputeForwardingAddr(GCObject* obj, void* data);

private:
    uint8_t* space_start_ = nullptr;    ///< 空间起始地址
    uint8_t* top_ = nullptr;            ///< 当前分配位置
    size_t capacity_ = 0;               ///< 总容量
    size_t used_size_ = 0;              ///< 已使用大小

    // 扩容期间使用的临时变量
    uint8_t* old_space_start_ = nullptr;  ///< 旧内存地址（扩容期间保留）
    uint8_t* old_top_ = nullptr;          ///< 旧内存的top位置
};

} // namespace mjs

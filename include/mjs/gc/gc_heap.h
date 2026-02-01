/**
 * @file gc_heap.h
 * @brief GC堆管理器定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了垃圾回收堆管理器，负责：
 * - 新生代内存管理（复制算法）
 * - 老年代内存管理（标记-压缩算法）
 * - 对象分配和晋升
 * - GC触发和调度
 */

#pragma once

#include <vector>
#include <memory>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/gc/gc_object.h>

namespace mjs {

class Context;
class Value;

/**
 * @brief 新生代半区大小（256KB）
 */
constexpr size_t kNewSpaceSemiSize = 256 * 1024;

/**
 * @brief 新生代总大小（512KB）
 */
constexpr size_t kNewSpaceSize = kNewSpaceSemiSize * 2;

/**
 * @brief 老年代初始大小（1MB）
 */
constexpr size_t kOldSpaceInitialSize = 1024 * 1024;

/**
 * @brief 对象晋升年龄阈值
 */
constexpr uint8_t kTenureAgeThreshold = 3;

/**
 * @brief 大对象阈值（直接在老年代分配）
 */
constexpr size_t kLargeObjectThreshold = kNewSpaceSemiSize / 4;

/**
 * @class NewSpace
 * @brief 新生代内存空间（使用复制算法）
 *
 * 新生代分为两个半区（From和To），使用复制算法进行GC。
 * 存活对象从From区复制到To区，然后交换两个半区。
 */
class NewSpace : public noncopyable {
public:
    /**
     * @brief 构造函数
     */
    NewSpace();
    
    /**
     * @brief 析构函数
     */
    ~NewSpace();

    /**
     * @brief 初始化新生代空间
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 分配内存（在From空间）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* Allocate(size_t size);

    /**
     * @brief 在To空间分配内存（GC复制时使用）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* AllocateInToSpace(size_t size);

    /**
     * @brief 检查是否有足够空间
     * @param size 需要的大小
     * @return 是否有足够空间
     */
    bool HasSpace(size_t size) const;

    /**
     * @brief 获取当前分配位置
     */
    uint8_t* top() const { return top_; }

    /**
     * @brief 获取From区起始地址
     */
    uint8_t* from_space() const { return from_space_; }

    /**
     * @brief 获取To区起始地址
     */
    uint8_t* to_space() const { return to_space_; }

    /**
     * @brief 获取To区当前分配位置
     */
    uint8_t* to_space_top() const { return to_space_top_; }

    /**
     * @brief 获取From区结束地址
     */
    uint8_t* from_space_end() const { return from_space_ + kNewSpaceSemiSize; }

    /**
     * @brief 交换From和To区
     */
    void SwapSpaces();

    /**
     * @brief 重置新生代（交换后使用）
     * @param new_top 新的top位置
     */
    void Reset(uint8_t* new_top);

    /**
     * @brief 重置To空间分配指针（GC开始时调用）
     */
    void ResetToSpace() {
        to_space_top_ = to_space_;
    }

    /**
     * @brief 遍历From区中的所有对象
     * @param callback 回调函数
     */
    void IterateObjects(std::function<void(GCObject*)> callback);

    /**
     * @brief 获取已使用内存大小
     */
    size_t used_size() const { return static_cast<size_t>(top_ - from_space_); }

    /**
     * @brief 获取总容量
     */
    static constexpr size_t capacity() { return kNewSpaceSemiSize; }

private:
    uint8_t* from_space_ = nullptr;     ///< From区（当前使用）
    uint8_t* to_space_ = nullptr;       ///< To区（复制目标）
    uint8_t* top_ = nullptr;            ///< From区当前分配位置
    uint8_t* to_space_top_ = nullptr;   ///< To区当前分配位置（GC时使用）
};

/**
 * @class OldSpace
 * @brief 老年代内存空间（使用标记-压缩算法）
 *
 * 老年代使用标记-压缩算法进行GC，减少内存碎片。
 * 支持动态扩容。
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
     * @param initial_size 初始大小
     * @return 是否初始化成功
     */
    bool Initialize(size_t initial_size = kOldSpaceInitialSize);

    /**
     * @brief 分配内存（在From空间）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* Allocate(size_t size);

    /**
     * @brief 在To空间分配内存（GC复制时使用）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* AllocateInToSpace(size_t size);

    /**
     * @brief 扩展老年代空间
     * @param min_size 最小扩展大小
     * @return 是否扩展成功
     */
    bool Expand(size_t min_size);

    /**
     * @brief 遍历所有对象
     * @param callback 回调函数
     */
    void IterateObjects(std::function<void(GCObject*)> callback);

    /**
     * @brief 遍历所有存活对象（已标记）
     * @param callback 回调函数
     */
    void IterateLiveObjects(std::function<void(GCObject*)> callback);

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

private:
    uint8_t* space_start_ = nullptr;    ///< 空间起始地址
    uint8_t* top_ = nullptr;            ///< 当前分配位置
    size_t capacity_ = 0;               ///< 总容量
    size_t used_size_ = 0;              ///< 已使用大小
};

/**
 * @struct GCRootSet
 * @brief GC根集合
 *
 * 存储所有GC根引用，包括：
 * - 栈上的值
 * - 全局对象
 * - 常量池中的对象
 */
struct GCRootSet {
    std::vector<Value*> stack_roots;           ///< 栈根
    std::vector<Value*> global_roots;          ///< 全局根
    std::vector<Value*> const_pool_roots;      ///< 常量池根
    std::vector<GCObject*> pinned_objects;     ///< 固定对象

    /**
     * @brief 清空所有根
     */
    void Clear() {
        stack_roots.clear();
        global_roots.clear();
        const_pool_roots.clear();
        pinned_objects.clear();
    }
};

/**
 * @class GCHeap
 * @brief GC堆管理器
 *
 * 统一管理新生代和老年代，提供对象分配、垃圾回收等功能。
 * 每个Context拥有一个独立的GCHeap。
 */
class GCHeap : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param context 所属上下文
     */
    explicit GCHeap(Context* context);
    
    /**
     * @brief 析构函数
     */
    ~GCHeap();

    /**
     * @brief 初始化堆
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 分配GC对象
     * @param type 对象类型
     * @param data_size 数据区域大小
     * @return GCObject指针，失败返回nullptr
     */
    GCObject* AllocateObject(GCObjectType type, size_t data_size);

    /**
     * @brief 分配指定大小的内存（用于特定类型）
     * @param type 对象类型
     * @param total_size 总大小（包含头部）
     * @return GCObject指针，失败返回nullptr
     */
    GCObject* Allocate(GCObjectType type, size_t total_size);

    /**
     * @brief 分配原始内存（不构造GCObject）
     * @param type 对象类型
     * @param total_size 总大小（包含头部）
     * @return 原始内存指针，失败返回nullptr
     * @note 调用者负责使用placement new构造对象
     */
    void* AllocateRaw(GCObjectType type, size_t total_size);

    /**
     * @brief 触发垃圾回收
     * @param full_gc 是否执行完全GC（包括老年代）
     * @return 是否回收成功
     */
    bool CollectGarbage(bool full_gc = false);

    /**
     * @brief 设置GC触发阈值
     * @param threshold 阈值（0-100，百分比）
     */
    void set_gc_threshold(uint8_t threshold) { gc_threshold_ = threshold; }

    /**
     * @brief 获取GC统计信息
     * @param total_allocated 总分配字节数
     * @param total_collected 总回收字节数
     * @param gc_count GC次数
     */
    void GetStats(size_t& total_allocated, size_t& total_collected, uint32_t& gc_count) const;

    /**
     * @brief 强制触发完整GC
     */
    void ForceFullGC();

    /**
     * @brief 添加根引用
     * @param value 值指针
     */
    void AddRoot(Value* value);

    /**
     * @brief 移除根引用
     * @param value 值指针
     */
    void RemoveRoot(Value* value);

private:
    /**
     * @brief 新生代GC（复制算法）
     * @return 是否回收成功
     */
    bool Scavenge();

    /**
     * @brief 老年代GC（标记-压缩算法）
     * @return 是否回收成功
     */
    bool MarkCompact();

    /**
     * @brief 标记阶段
     */
    void MarkPhase();

    /**
     * @brief 压缩阶段
     */
    void CompactPhase();

    /**
     * @brief 复制对象到To空间
     * @param obj 要复制的对象
     * @return 新地址
     */
    GCObject* CopyObject(GCObject* obj);

    /**
     * @brief 晋升对象到老年代
     * @param obj 要晋升的对象
     * @return 新地址，失败返回nullptr
     */
    GCObject* PromoteObject(GCObject* obj);

    /**
     * @brief 从根集合开始标记
     */
    void MarkFromRoots();

    /**
     * @brief 标记一个对象及其子对象
     * @param obj 要标记的对象
     */
    void MarkObject(GCObject* obj);

    /**
     * @brief 处理写屏障（跨代引用）
     * @param parent 父对象
     * @param child 子对象
     */
    void WriteBarrier(GCObject* parent, GCObject* child);

    /**
     * @brief 收集根集合
     */
    void CollectRoots();

private:
    Context* context_ = nullptr;           ///< 所属上下文
    std::unique_ptr<NewSpace> new_space_;  ///< 新生代空间
    std::unique_ptr<OldSpace> old_space_;  ///< 老年代空间
    
    GCRootSet root_set_;                   ///< GC根集合
    
    // GC统计
    size_t total_allocated_ = 0;           ///< 总分配字节数
    size_t total_collected_ = 0;           ///< 总回收字节数
    uint32_t gc_count_ = 0;                ///< GC次数
    uint32_t full_gc_count_ = 0;           ///< 完整GC次数
    
    // GC配置
    uint8_t gc_threshold_ = 80;            ///< GC触发阈值（百分比）
    bool in_gc_ = false;                   ///< 是否正在进行GC
};

} // namespace mjs

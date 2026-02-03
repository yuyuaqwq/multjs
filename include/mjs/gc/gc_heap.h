/**
 * @file gc_heap.h
 * @brief GC堆管理器定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了垃圾回收堆管理器，负责：
 * - 统一管理新生代和老年代
 * - 对象分配和晋升
 * - GC触发和调度
 */

#pragma once

#include <vector>
#include <memory>
#include <unordered_set>

#include <mjs/noncopyable.h>
#include <mjs/gc/gc_object.h>
#include <mjs/gc/new_space.h>
#include <mjs/gc/old_space.h>
#include <mjs/value/value.h>

namespace mjs {

class Context;

/**
 * @brief 新生代总大小（512KB）
 */
constexpr size_t kNewSpaceSize = kNewSpaceSemiSize * 2;

/**
 * @brief 对象晋升年龄阈值
 */
constexpr uint8_t kTenureAgeThreshold = 3;

/**
 * @brief 大对象阈值（直接在老年代分配）
 */
constexpr size_t kLargeObjectThreshold = kNewSpaceSemiSize / 4;

/**
 * @struct GCRootSet
 * @brief GC根集合
 *
 * 仅存储用户手动添加的永久根引用。
 * 栈、微任务队列等临时根在 GC 时通过 IterateRoots 直接处理，不存储。
 */
struct GCRootSet {
    std::unordered_set<Value*> global_roots;          ///< 用户手动添加的全局根
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
     * @brief 分配原始内存（不构造GCObject）
     * @param total_size 总大小（包含头部）
     * @param generation 输出代际信息
     * @return 原始内存指针，失败返回nullptr
     * @note 调用者负责使用placement new构造对象
     */
    void* Allocate(size_t* total_size, GCGeneration* generation);

    /**
     * @brief 触发垃圾回收
     * @param full_gc 是否执行完全GC（包括老年代）
     * @return 是否回收成功
     */
    bool CollectGarbage(bool full_gc = false);

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

    /**
     * @brief 设置GC触发阈值
     * @param threshold 阈值（0-100，百分比）
     */
    void set_gc_threshold(uint8_t threshold) { gc_threshold_ = threshold; }

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
     * @brief 处理对象拷贝（Scavenge时使用）
     * @param value 值指针
     */
    void ProcessCopyOrReference(Value* value);

    /**
     * @brief 扩展老年代空间
     * @param min_size 最小需要的额外空间
     * @return 是否扩容成功
     */
    bool ExpandOldSpace(size_t min_size);

    /**
     * @brief 根回调函数类型
     * @param value 根值指针
     * @param data 用户数据
     */
    using RootCallback = void (*)(Value* root, void* data);

    /**
     * @brief 遍历所有根并调用回调（参考 Java 的 OopClosure 模式）
     * @param callback 回调函数
     * @param data 用户数据（通常传递 this）
     */
    void IterateRoots(RootCallback callback, void* data);

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

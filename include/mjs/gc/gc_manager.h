/**
 * @file gc_manager.h
 * @brief JavaScript 垃圾回收管理器
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的垃圾回收管理器，负责：
 * - 统一内存分配管理
 * - 分代垃圾回收（新生代 + 老年代）
 * - 复制清除算法（新生代）
 * - 标记压缩算法（老年代）
 * - 对象晋升和跨代引用管理
 * 
 * 针对Actor模型设计，每个Context拥有独立的GCHeap。
 */

#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <mjs/noncopyable.h>
#include <mjs/gc/gc_heap.h>

namespace mjs {

class Object;
class Value;
class Context;

/**
 * @class GCManager
 * @brief 垃圾回收管理器类
 *
 * 统一管理JavaScript引擎中的内存分配和垃圾回收：
 * - 提供统一的内存分配接口
 * - 管理新生代和老年代的GC
 * - 追踪GC统计信息
 * - 支持手动触发GC
 *
 * @note 每个Context拥有一个独立的GCManager实例
 * @see GCHeap
 * @see GCObject
 */
class GCManager : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param context 所属上下文
     */
    explicit GCManager(Context* context);

    /**
     * @brief 析构函数
     */
    ~GCManager();

    /**
     * @brief 初始化GC管理器
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 分配对象内存
     * @tparam ObjectT 对象类型
     * @tparam Args 构造函数参数类型
     * @param gc_type GC对象类型
     * @param args 构造函数参数
     * @return 分配的对象指针，失败返回nullptr
     */
    template<typename ObjectT, typename... Args>
    ObjectT* AllocateObject(GCObjectType gc_type, Args&&... args) {
        size_t data_size = sizeof(ObjectT);
        GCObject* gc_obj = heap_->AllocateObject(gc_type, data_size);
        if (!gc_obj) {
            return nullptr;
        }
        
        // 在GCObject的数据区域构造对象
        void* data = gc_obj->data();
        return new (data) ObjectT(std::forward<Args>(args)...);
    }

    /**
     * @brief 分配指定大小的内存
     * @param gc_type GC对象类型
     * @param size 大小（包含头部）
     * @return GCObject指针，失败返回nullptr
     */
    GCObject* Allocate(GCObjectType gc_type, size_t size);

    /**
     * @brief 执行垃圾回收
     * @param full_gc 是否执行完整GC（包括老年代）
     * @return 是否回收成功
     */
    bool CollectGarbage(bool full_gc = false);

    /**
     * @brief 强制触发完整GC
     */
    void ForceFullGC();

    /**
     * @brief 获取GC堆统计信息
     * @param new_space_used 新生代已使用大小
     * @param new_space_capacity 新生代容量
     * @param old_space_used 老年代已使用大小
     * @param old_space_capacity 老年代容量
     */
    void GetHeapStats(size_t& new_space_used, size_t& new_space_capacity,
                      size_t& old_space_used, size_t& old_space_capacity) const;

    /**
     * @brief 获取GC统计信息
     * @param total_allocated 总分配字节数
     * @param total_collected 总回收字节数
     * @param gc_count GC次数
     */
    void GetGCStats(size_t& total_allocated, size_t& total_collected, uint32_t& gc_count) const;

    /**
     * @brief 打印GC统计信息
     */
    void PrintStats() const;

    /**
     * @brief 设置GC触发阈值
     * @param threshold 阈值（0-100，百分比）
     */
    void SetGCThreshold(uint8_t threshold);

    /**
     * @brief 获取GC堆
     * @return GC堆指针
     */
    GCHeap* heap() const { return heap_.get(); }

    /**
     * @brief 添加GC根引用（用于持久引用）
     * @param value 值指针
     */
    void AddRoot(Value* value);

    /**
     * @brief 移除GC根引用
     * @param value 值指针
     */
    void RemoveRoot(Value* value);

    // ========== 兼容性接口（旧API）==========

    /**
     * @brief 添加对象到垃圾回收管理器（旧API，已弃用）
     * @param object 对象指针
     * @note 新GC系统中对象自动管理，此方法仅用于兼容
     */
    void AddObject(Object* object);

    /**
     * @brief 执行垃圾回收（旧API）
     * @param context 执行上下文指针
     * @note 新GC系统不需要context参数，使用新的CollectGarbage方法
     */
    void GC(Context* context);

    /**
     * @brief 打印对象树结构
     * @param context 执行上下文指针
     */
    void PrintObjectTree(Context* context);

private:
    Context* context_ = nullptr;               ///< 所属上下文
    std::unique_ptr<GCHeap> heap_ = nullptr;   ///< GC堆
};

} // namespace mjs

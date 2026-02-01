/**
 * @file gc_allocator.h
 * @brief GC内存分配器接口
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了GC内存分配器接口，用于统一分配所有GC管理的内存。
 * 支持分代垃圾回收、复制清除和标记压缩算法。
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <mjs/noncopyable.h>

namespace mjs {

class Context;
class GCHeap;
class GCObject;

/**
 * @enum GCAllocType
 * @brief 内存分配类型
 */
enum class GCAllocType : uint8_t {
    kObject = 0,           ///< 普通JavaScript对象
    kArray,                ///< 数组对象
    kFunction,             ///< 函数对象
    kString,               ///< 字符串对象
    kShape,                ///< 形状对象
    kModuleDef,            ///< 模块定义
    kFunctionDef,          ///< 函数定义
    kClosureVar,           ///< 闭包变量
    kOther,                ///< 其他类型
};

/**
 * @class GCAllocator
 * @brief GC内存分配器
 *
 * 提供统一的内存分配接口，所有GC管理的对象都应该通过此分配器分配。
 * 支持对象对齐和大小类别优化。
 */
class GCAllocator : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param context 所属上下文
     */
    explicit GCAllocator(Context* context);
    
    /**
     * @brief 析构函数
     */
    ~GCAllocator();

    /**
     * @brief 初始化分配器
     * @return 是否初始化成功
     */
    bool Initialize();

    /**
     * @brief 分配内存
     * @param type 分配类型
     * @param size 分配大小（字节）
     * @return 分配的内存地址，失败返回nullptr
     */
    void* Allocate(GCAllocType type, size_t size);

    /**
     * @brief 释放内存（通常由GC自动处理）
     * @param ptr 内存地址
     */
    void Free(void* ptr);

    /**
     * @brief 触发垃圾回收
     * @param full_gc 是否执行完整GC
     * @return 是否回收成功
     */
    bool Collect(bool full_gc = false);

    /**
     * @brief 强制触发完整GC
     */
    void ForceFullCollection();

    /**
     * @brief 获取堆统计信息
     * @param used 已使用内存
     * @param capacity 总容量
     */
    void GetHeapStats(size_t& used, size_t& capacity) const;

    /**
     * @brief 获取GC统计信息
     * @param allocated 总分配字节数
     * @param collected 总回收字节数
     * @param gc_count GC次数
     */
    void GetStats(size_t& allocated, size_t& collected, uint32_t& gc_count) const;

    /**
     * @brief 设置GC阈值
     * @param threshold 阈值（0-100，百分比）
     */
    void SetThreshold(uint8_t threshold);

    /**
     * @brief 获取内部GC堆（用于高级操作）
     */
    GCHeap* heap() const { return heap_.get(); }

private:
    Context* context_ = nullptr;
    std::unique_ptr<GCHeap> heap_;
};

/**
 * @brief 全局分配函数（用于对象创建）
 * @param context 执行上下文
 * @param type 分配类型
 * @param size 分配大小
 * @return 分配的内存地址
 */
void* GCAllocate(Context* context, GCAllocType type, size_t size);

/**
 * @brief 全局释放函数
 * @param ptr 内存地址
 */
void GCFree(void* ptr);

/**
 * @brief 触发GC
 * @param context 执行上下文
 * @param full_gc 是否执行完整GC
 * @return 是否回收成功
 */
bool GCCollect(Context* context, bool full_gc = false);

} // namespace mjs

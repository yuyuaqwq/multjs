/**
 * @file new_space.h
 * @brief 新生代内存空间定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了新生代内存空间（NewSpace），使用复制算法进行GC。
 */

#pragma once

#include <mjs/noncopyable.h>
#include <mjs/gc/gc_object.h>

namespace mjs {

/**
 * @brief 新生代半区大小（256KB）
 */
constexpr size_t kNewSpaceSemiSize = 256 * 1024;

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
    void* Allocate(size_t* size);

    /**
     * @brief 在To空间分配内存（GC复制时使用）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* AllocateInToSpace(size_t* size);

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
     * @brief 对象回调函数类型
     * @param obj 对象指针
     * @param data 用户数据
     */
    using ObjectCallback = void (*)(GCObject* obj, void* data);

    /**
     * @brief 遍历From区中的所有对象
     * @param callback 回调函数
     * @param data 用户数据
     */
    void IterateObjects(ObjectCallback callback, void* data);

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

} // namespace mjs

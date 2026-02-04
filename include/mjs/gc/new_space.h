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
 * @brief 新生代总大小（512KB）
 */
constexpr size_t kNewSpaceTotalSize = 512 * 1024;

/**
 * @brief Eden区大小比例（80%）
 */
constexpr size_t kEdenSpaceRatio = 8;

/**
 * @brief Survivor区大小比例（各10%）
 */
constexpr size_t kSurvivorSpaceRatio = 1;

/**
 * @brief 总比例份数（8+1+1=10）
 */
constexpr size_t kTotalSpaceRatio = 10;

/**
 * @brief Eden区大小
 */
constexpr size_t kEdenSpaceSize = kNewSpaceTotalSize * kEdenSpaceRatio / kTotalSpaceRatio;

/**
 * @brief Survivor区大小（每个）
 */
constexpr size_t kSurvivorSpaceSize = kNewSpaceTotalSize * kSurvivorSpaceRatio / kTotalSpaceRatio;

/**
 * @class NewSpace
 * @brief 新生代内存空间（使用复制算法）
 *
 * 新生代分为三个区域：Eden、Survivor From、Survivor To。
 * - Eden区：新对象分配区域，占用大部分空间（80%）
 * - Survivor From：存放上次GC后存活的对象（10%）
 * - Survivor To：存放本次GC后存活的对象（10%）
 *
 * GC过程：将Eden和Survivor From中的存活对象复制到Survivor To，然后交换From和To。
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
     * @brief 分配内存（在Eden区）
     * @param size 分配大小
     * @return 分配的内存地址，失败返回nullptr
     */
    void* Allocate(size_t* size);

    /**
     * @brief 在Survivor To区分配内存（GC复制时使用）
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
     * @brief 获取Eden区当前分配位置
     */
    uint8_t* eden_top() const { return eden_top_; }

    /**
     * @brief 获取Eden区起始地址
     */
    uint8_t* eden_space() const { return eden_space_; }

    /**
     * @brief 获取Eden区结束地址
     */
    uint8_t* eden_space_end() const { return eden_space_ + kEdenSpaceSize; }

    /**
     * @brief 获取Survivor From区起始地址
     */
    uint8_t* survivor_from() const { return survivor_from_; }

    /**
     * @brief 获取Survivor From区结束地址
     */
    uint8_t* survivor_from_end() const { return survivor_from_ + kSurvivorSpaceSize; }

    /**
     * @brief 获取Survivor From区当前分配位置
     */
    uint8_t* survivor_from_top() const { return survivor_from_top_; }

    /**
     * @brief 获取Survivor To区起始地址
     */
    uint8_t* survivor_to() const { return survivor_to_; }

    /**
     * @brief 获取Survivor To区结束地址
     */
    uint8_t* survivor_to_end() const { return survivor_to_ + kSurvivorSpaceSize; }

    /**
     * @brief 获取Survivor To区当前分配位置
     */
    uint8_t* survivor_to_top() const { return survivor_to_top_; }

    /**
     * @brief 交换Survivor From和To区
     */
    void SwapSurvivorSpaces();

    /**
     * @brief 重置Eden区（GC后清空）
     */
    void ResetEden() {
        eden_top_ = eden_space_;
    }

    /**
     * @brief 重置Survivor To空间分配指针（GC开始时调用）
     */
    void ResetToSpace() {
        survivor_to_top_ = survivor_to_;
    }

    /**
     * @brief 对象回调函数类型
     * @param obj 对象指针
     * @param data 用户数据
     */
    using ObjectCallback = void (*)(GCObject* obj, void* data);

    /**
     * @brief 遍历Eden区和Survivor From区中的所有对象
     * @param callback 回调函数
     * @param data 用户数据
     */
    void IterateObjects(ObjectCallback callback, void* data);

    /**
     * @brief 获取已使用内存大小（Eden + Survivor From）
     */
    size_t used_size() const {
        return static_cast<size_t>(eden_top_ - eden_space_) +
               static_cast<size_t>(survivor_from_top_ - survivor_from_);
    }

    /**
     * @brief 获取总容量
     */
    static constexpr size_t capacity() { return kNewSpaceTotalSize; }

private:
    uint8_t* eden_space_ = nullptr;         ///< Eden区（新对象分配）
    uint8_t* survivor_from_ = nullptr;      ///< Survivor From区（上次GC存活对象）
    uint8_t* survivor_to_ = nullptr;        ///< Survivor To区（本次GC复制目标）
    uint8_t* eden_top_ = nullptr;           ///< Eden区当前分配位置
    uint8_t* survivor_from_top_ = nullptr;  ///< Survivor From区当前分配位置
    uint8_t* survivor_to_top_ = nullptr;    ///< Survivor To区当前分配位置（GC时使用）
};

} // namespace mjs

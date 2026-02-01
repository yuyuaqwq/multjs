/**
 * @file gc_object.h
 * @brief GC管理对象基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了垃圾回收管理对象的基类，所有需要GC管理的对象都必须继承自此基类。
 * 支持分代垃圾回收、复制清除和标记压缩算法。
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>

namespace mjs {

class Context;
class Value;
class GCHeap;

using GCTraverseCallback = void(*)(Context* context, Value& child);

/**
 * @enum GCGeneration
 * @brief GC对象所处代际枚举
 */
enum class GCGeneration : uint8_t {
    kNew = 0,       ///< 新生代
    kOld = 1,       ///< 老年代
};

/**
 * @enum GCObjectType
 * @brief GC对象类型枚举
 */
enum class GCObjectType : uint8_t {
    kObject = 0,           ///< 普通对象
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
 * @struct GCObjectHeader
 * @brief GC对象头部结构
 * 
 * 每个GC对象在内存中的头部信息，用于GC管理和追踪
 */
struct GCObjectHeader {
    union {
        uint32_t word_ = 0;     ///< 完整32位值
        struct {
            uint32_t type_ : 8;          ///< 对象类型 (GCObjectType)
            uint32_t generation_ : 1;    ///< 所在代际 (0=新生代, 1=老年代)
            uint32_t marked_ : 1;        ///< 标记位（用于标记-清除/标记-压缩）
            uint32_t forwarded_ : 1;     ///< 转发位（用于复制算法）
            uint32_t pinned_ : 1;        ///< 固定标记（不移动）
            uint32_t age_ : 4;           ///< 年龄（用于晋升判断）
            uint32_t size_class_ : 8;    ///< 大小类别（用于快速分配）
            uint32_t reserved_ : 8;      ///< 保留位
        };
    };
    uint32_t size_ = 0;              ///< 对象总大小（包含头部）
    
    /**
     * @brief 获取对象类型
     */
    GCObjectType type() const { return static_cast<GCObjectType>(type_); }
    
    /**
     * @brief 设置对象类型
     */
    void set_type(GCObjectType t) { type_ = static_cast<uint8_t>(t); }
    
    /**
     * @brief 获取代际
     */
    GCGeneration generation() const { return static_cast<GCGeneration>(generation_); }
    
    /**
     * @brief 设置代际
     */
    void set_generation(GCGeneration g) { generation_ = static_cast<uint8_t>(g); }
    
    /**
     * @brief 检查是否已标记
     */
    bool IsMarked() const { return marked_; }
    
    /**
     * @brief 设置标记
     */
    void SetMarked(bool m) { marked_ = m; }
    
    /**
     * @brief 检查是否已转发（复制算法中使用）
     */
    bool IsForwarded() const { return forwarded_; }
    
    /**
     * @brief 设置转发标记
     */
    void SetForwarded(bool f) { forwarded_ = f; }
    
    /**
     * @brief 检查是否固定
     */
    bool IsPinned() const { return pinned_; }
    
    /**
     * @brief 设置固定标记
     */
    void SetPinned(bool p) { pinned_ = p; }
    
    /**
     * @brief 获取年龄
     */
    uint8_t age() const { return static_cast<uint8_t>(age_); }
    
    /**
     * @brief 增加年龄
     */
    void IncrementAge() { ++age_; }
    
    /**
     * @brief 清空年龄
     */
    void ClearAge() { age_ = 0; }
};

/**
 * @class GCObject
 * @brief GC管理对象基类
 *
 * 所有需要垃圾回收管理的对象都必须继承自此基类。
 * 提供GC所需的基本功能和接口。
 */
class GCObject : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param type 对象类型
     * @param size 对象大小（包含头部）
     */
    explicit GCObject(GCObjectType type, size_t size);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~GCObject() = default;

    /**
     * @brief 遍历对象的子引用（用于GC追踪）
     * @param context 执行上下文
     * @param callback 回调函数，用于处理每个子引用
     * @note 所有包含引用的子对象必须实现此方法
     */
    virtual void GCTraverse(Context* context, GCTraverseCallback callback);

    /**
     * @brief 对象被移动后的回调（用于标记-压缩算法）
     * @param old_addr 对象原来的地址
     * @note 如果对象包含自引用指针，需要重写此方法进行修正
     */
    virtual void GCMoved(void* old_addr) {}

    /**
     * @brief 获取对象头部
     */
    GCObjectHeader* header() { return &header_; }
    
    /**
     * @brief 获取对象头部（常量）
     */
    const GCObjectHeader* header() const { return &header_; }

    /**
     * @brief 获取对象类型
     */
    GCObjectType gc_type() const { return header_.type(); }
    
    /**
     * @brief 获取对象大小
     */
    size_t gc_size() const { return header_.size_; }

    /**
     * @brief 获取对象数据区域起始地址
     */
    void* data() { return reinterpret_cast<uint8_t*>(this) + sizeof(GCObjectHeader); }
    
    /**
     * @brief 获取对象数据区域起始地址（常量）
     */
    const void* data() const { return reinterpret_cast<const uint8_t*>(this) + sizeof(GCObjectHeader); }

    /**
     * @brief 从数据指针获取GCObject
     * @param data 数据区域指针
     * @return GCObject指针
     */
    static GCObject* FromData(void* data) {
        return reinterpret_cast<GCObject*>(
            reinterpret_cast<uint8_t*>(data) - sizeof(GCObjectHeader));
    }

    /**
     * @brief 从GCObject指针获取数据指针
     * @param obj GCObject指针
     * @return 数据区域指针
     */
    static void* ToData(GCObject* obj) {
        return reinterpret_cast<uint8_t*>(obj) + sizeof(GCObjectHeader);
    }

protected:
    GCObjectHeader header_;     ///< 对象头部
};

/**
 * @brief GC对象最小对齐大小
 */
constexpr size_t kGCObjectAlignment = 8;

/**
 * @brief 对齐对象大小
 * @param size 原始大小
 * @return 对齐后的大小
 */
inline size_t AlignGCObjectSize(size_t size) {
    return (size + kGCObjectAlignment - 1) & ~(kGCObjectAlignment - 1);
}

/**
 * @brief 计算包含头部的总大小
 * @param data_size 数据区域大小
 * @return 总大小（包含头部和对齐）
 */
inline size_t GCObjectTotalSize(size_t data_size) {
    return AlignGCObjectSize(sizeof(GCObjectHeader) + data_size);
}

} // namespace mjs

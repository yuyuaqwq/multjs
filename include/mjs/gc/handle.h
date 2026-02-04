/**
 * @file handle.h
 * @brief 栈上句柄和句柄作用域
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了栈上句柄机制，用于保护新分配的对象在初始化期间不被 GC 回收。
 *
 * 设计思路：
 * - 使用模板参数指定句柄容量，编译期确定大小
 * - 使用静态数组存储 GCObject*，零堆分配
 * - 适用于大多数场景（容量通常在 1-16 之间）
 * - 直接存储 GCObject*，节省内存并优化 GC 扫描
 */

#pragma once

#include <array>

#include <mjs/noncopyable.h>
#include <mjs/value/value.h>
#include <mjs/gc/gc_object.h>

namespace mjs {

class Context;
class GCManager;

/**
 * @class HandleScopeBase
 * @brief HandleScope 的非模板基类，用于类型擦除
 *
 * 提供了 Context 管理句柄作用域所需的公共接口。
 */
class GCHandleScopeBase : public noncopyable {
public:
    virtual ~GCHandleScopeBase() = default;

    /**
     * @brief 获取上一个作用域
     */
    GCHandleScopeBase* prev() const { return prev_; }

    /**
     * @brief 设置上一个作用域
     */
    void set_prev(GCHandleScopeBase* prev) { prev_ = prev; }

    /**
     * @brief 获取当前句柄数量
     */
    virtual size_t size() const = 0;

    /**
     * @brief 获取句柄数据（供 GC 扫描）
     * @return 指向 GCObject* 数组的指针
     */
    virtual const GCObject* const* data() const = 0;

    /**
     * @brief 标记作用域为已分离（Close 时使用）
     */
    void mark_detached() { scope_detached_ = true; }

    /**
     * @brief 检查是否已分离
     */
    bool is_detached() const { return scope_detached_; }

    /**
     * @brief 获取所属的 Context
     */
    Context* context() const { return context_; }

protected:
    GCHandleScopeBase(Context* context)
        : context_(context), prev_(nullptr), scope_detached_(false) {}

    Context* context_;          ///< 所属上下文
    GCHandleScopeBase* prev_;     ///< 上一个作用域
    bool scope_detached_;       ///< 是否已提前弹出
};

/**
 * @class Handle
 * @brief 栈上句柄，保护对象不被 GC 回收
 *
 * @tparam T 对象类型（如 ArrayObject, Object 等）
 *
 * Handle 直接存储 GCObject*，避免额外的 Value 包装。
 */
template<typename T>
class GCHandle {
public:
    GCHandle() : obj_(nullptr) {}

    GCHandle(GCObject* obj) : obj_(obj) {}

    /**
     * @brief 访问对象的成员
     */
    T* operator->() const {
        return static_cast<T*>(obj_);
    }

    /**
     * @brief 解引用对象
     */
    T& operator*() const {
        return *operator->();
    }

    /**
     * @brief 转换为 Value
     */
    Value ToValue() const {
        return Value(static_cast<T*>(obj_));
    }

    /**
     * @brief 检查句柄是否为空
     */
    bool IsEmpty() const {
        return obj_ == nullptr;
    }

    /**
     * @brief 获取底层 GCObject*
     */
    GCObject* gc_obj() const { return obj_; }

private:
    GCObject* obj_;       ///< GC 对象指针

    template<size_t>
    friend class HandleScope;
};

/**
 * @class HandleScope
 * @brief 句柄作用域（模板版本，零堆分配）
 *
 * @tparam Capacity 句柄容量，编译期确定
 *
 * 使用静态数组存储 GCObject*，完全避免堆分配。
 * 适合大多数场景，容量通常在 1-16 之间。
 *
 * 使用示例：
 * @code
 *   // 使用默认容量（8个句柄）
 *   Value SomeFunction(Context* context) {
 *       HandleScope<8> scope(context);
 *
 *       // 推荐：使用 New 直接分配对象
 *       auto arr = scope.New<ArrayObject>(10);
 *       auto obj = scope.New<Object>();
 *
 *       // 或者保护已有的对象
 *       auto existing = context->gc_manager().AllocateObject<ArrayObject>(5);
 *       auto handle = scope.Create(existing);
 *
 *       return scope.Close(arr);
 *   }
 *
 *   // 自定义容量
 *   Value ComplexFunction(Context* context) {
 *       HandleScope<16> scope(context);  // 需要 16 个句柄
 *       auto obj1 = scope.New<Object>();
 *       auto obj2 = scope.New<ArrayObject>(100);
 *       // ... 最多创建 16 个句柄
 *   }
 * @endcode
 *
 * @note 如果容量不足，会触发断言失败
 * @see Handle
 */
template<size_t Capacity>
class GCHandleScope : public GCHandleScopeBase {
public:
    static_assert(Capacity > 0, "HandleScope capacity must be greater than 0");

    /**
     * @brief 构造函数
     * @param context 执行上下文
     */
    explicit GCHandleScope(Context* context)
        : GCHandleScopeBase(context), size_(0) {
        context_->PushHandleScope(this);
    }

    /**
     * @brief 析构函数
     */
    ~GCHandleScope() override {
        // 如果已经通过 Close() 弹出，则不再弹出
        if (!scope_detached_) {
            context_->PopHandleScope();
        }
    }

    /**
     * @brief 分配对象并创建句柄（推荐使用）
     * @tparam T 对象类型
     * @tparam Args 构造参数类型
     * @param args 构造参数
     * @return Handle<T> 句柄
     *
     * 这是推荐的分配对象方式，会自动分配对象并创建句柄保护。
     *
     * @code
     *   HandleScope<8> scope(context);
     *   auto arr = scope.New<ArrayObject>(10);  // 分配 ArrayObject
     *   auto obj = scope.New<Object>();          // 分配 Object
     * @endcode
     *
     * @assert 如果容量不足会触发断言失败
     */
    template<typename T, typename... Args>
    GCHandle<T> New(Args&&... args) {
        // 编译期和运行期双重检查
        assert(size_ < Capacity && "HandleScope capacity exceeded");

        // 分配对象
        GCManager& gc_mgr = context_->gc_manager();
        T* ptr = gc_mgr.template AllocateObject<T>(std::forward<Args>(args)...);

        // 将 Object* 转换为 GCObject*
        GCObject* gc_obj = static_cast<GCObject*>(ptr);

        handles_[size_++] = gc_obj;
        return GCHandle<T>(gc_obj);
    }

    /**
     * @brief 创建一个新句柄（从已有对象）
     * @tparam T 对象类型
     * @param ptr 对象指针
     * @return Handle<T> 句柄
     *
     * 用于保护已分配的对象。
     *
     * @assert 如果容量不足会触发断言失败
     */
    template<typename T>
    GCHandle<T> Create(T* ptr) {
        // 编译期和运行期双重检查
        assert(size_ < Capacity && "HandleScope capacity exceeded");

        GCObject* gc_obj = static_cast<GCObject*>(ptr);

        handles_[size_++] = gc_obj;
        return GCHandle<T>(gc_obj);
    }

    /**
     * @brief 关闭作用域并返回 Value
     * @tparam T 句柄类型
     * @param handle 要返回的句柄
     * @return Value 值
     *
     * 显式表达所有权转移，在返回之前会弹出 HandleScope，但保留返回的 Value。
     */
    template<typename T>
    Value Close(const GCHandle<T>& handle) {
        Value result = handle.ToValue();
        // 提前弹出作用域，避免 ~HandleScope() 再次弹出
        context_->PopHandleScope();
        scope_detached_ = true;
        return result;
    }

    /**
     * @brief 获取所有句柄（供 GC 扫描）
     */
    const std::array<GCObject*, Capacity>& handles() const { return handles_; }

    /**
     * @brief 获取当前句柄数量
     */
    size_t size() const override { return size_; }

    /**
     * @brief 获取句柄数据（供 GC 扫描）
     */
    const GCObject* const* data() const override { return handles_.data(); }

    /**
     * @brief 获取容量
     */
    static constexpr size_t capacity() { return Capacity; }

private:
    size_t size_;                                   ///< 当前句柄数量
    std::array<GCObject*, Capacity> handles_;       ///< 静态数组存储 GCObject*
};

/**
 * @brief 默认句柄容量
 *
 * 大多数函数只需要 1-8 个句柄，这个默认值适合 99% 的场景。
 */
inline constexpr size_t kDefaultHandleScopeCapacity = 8;

/**
 * @brief 类型别名，使用默认容量
 */
using DefaultHandleScope = GCHandleScope<kDefaultHandleScopeCapacity>;

} // namespace mjs

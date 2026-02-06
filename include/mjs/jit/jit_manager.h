/**
 * @file jit_manager.h
 * @brief JIT管理器
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了JIT管理器，负责协调JIT编译流程和代码缓存管理
 */

#pragma once

#include <memory>
#include <unordered_map>

#ifdef ENABLE_JIT
#include <asmjit/core/jitruntime.h>
#endif

namespace mjs {

class Context;
class Value;
namespace jit {
class JitCode;
}

class FunctionDefBase;

namespace jit {

/**
 * @class JITManager
 * @brief JIT管理器
 *
 * 负责协调JIT编译流程，管理代码缓存
 */
class JITManager {
public:
    /**
     * @brief 构造函数
     * @param context 执行上下文指针
     */
    explicit JITManager(Context* context);

    /**
     * @brief 析构函数
     */
    ~JITManager();

    // 禁止拷贝和移动
    JITManager(const JITManager&) = delete;
    JITManager& operator=(const JITManager&) = delete;
    JITManager(JITManager&&) = delete;
    JITManager& operator=(JITManager&&) = delete;

    /**
     * @brief 编译函数为Baseline JIT代码
     * @param func_def 函数定义指针
     *
     * @note 如果函数已经被编译，将直接返回缓存的代码
     */
    void CompileBaseline(FunctionDefBase* func_def);

    /**
     * @brief 获取函数的Baseline JIT代码
     * @param func_def 函数定义指针
     * @return JIT代码指针，如果未编译则返回nullptr
     */
    JitCode* GetBaselineCode(FunctionDefBase* func_def);

    /**
     * @brief 清理代码缓存
     * @param max_size 最大缓存大小（字节）
     *
     * 如果缓存超过指定大小，将使用LRU策略清理
     */
    void PruneCache(size_t max_size);

    /**
     * @brief 获取当前缓存大小
     * @return 缓存大小（字节）
     */
    size_t GetCacheSize() const noexcept {
        return total_cache_size_;
    }

    /**
     * @brief 获取缓存的代码数量
     * @return 代码数量
     */
    size_t GetCacheCount() const noexcept {
        return code_cache_.size();
    }

#ifdef ENABLE_JIT
    /**
     * @brief 获取 JIT 运行时
     * @return JIT 运行时引用
     */
    asmjit::JitRuntime& runtime() noexcept {
        return runtime_;
    }
#endif

private:
    /**
     * @brief 执行实际的编译工作
     * @param func_def 函数定义指针
     * @return 编译后的JIT代码
     */
    std::unique_ptr<JitCode> CompileBaselineImpl(FunctionDefBase* func_def);

    Context* context_;  ///< 执行上下文

#ifdef ENABLE_JIT
    asmjit::JitRuntime runtime_;  ///< JIT 运行时
#endif

    /// 代码缓存：函数定义 -> JIT代码
    std::unordered_map<FunctionDefBase*, std::unique_ptr<JitCode>> code_cache_;

    /// LRU列表（用于缓存清理）
    std::unordered_map<FunctionDefBase*, uint64_t> lru_timestamps_;

    /// 当前缓存总大小
    size_t total_cache_size_;

    /// LRU时间戳计数器
    uint64_t lru_counter_;
};

} // namespace jit
} // namespace mjs

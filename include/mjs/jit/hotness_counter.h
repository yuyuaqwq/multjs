/**
 * @file hotness_counter.h
 * @brief JIT热点计数器
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了热点计数器，用于跟踪函数执行次数并触发JIT编译
 */

#pragma once

#include <cstdint>

namespace mjs::jit {

/**
 * @brief 执行状态枚举
 */
enum class ExecutionState : uint8_t {
    kInterpreted,   // 解释执行中
    kWarmup,        // 需要Baseline编译
    kBaseline,      // Baseline JIT执行中
    kHot,           // 需要Optimized编译（预留）
    kOptimized,     // Optimized JIT执行中（预留）
};

/**
 * @class HotnessCounter
 * @brief 热点计数器类
 *
 * 跟踪函数执行次数，管理执行状态，触发JIT编译
 */
class HotnessCounter {
public:
    /**
     * @brief Baseline JIT触发阈值
     *
     * 函数执行达到100次后触发Baseline JIT编译
     */
    static constexpr uint32_t kBaselineThreshold = 100;

    /**
     * @brief Optimized JIT触发阈值（预留）
     *
     * 函数执行达到10000次后触发Optimized JIT编译
     */
    static constexpr uint32_t kOptimizedThreshold = 10000;

    /**
     * @brief 构造函数
     */
    HotnessCounter() noexcept
        : count_(0), state_(ExecutionState::kInterpreted) {}

    /**
     * @brief 增加执行计数
     *
     * 每次函数调用时调用，自动检测是否达到阈值
     */
    void Increment() noexcept {
        if (state_ != ExecutionState::kOptimized) {
            count_++;
            CheckThreshold();
        }
    }

    /**
     * @brief 重置计数器
     */
    void Reset() noexcept {
        count_ = 0;
    }

    /**
     * @brief 获取当前执行状态
     * @return 执行状态
     */
    ExecutionState state() const noexcept {
        return state_;
    }

    /**
     * @brief 设置执行状态
     * @param state 新的执行状态
     */
    void set_state(ExecutionState state) noexcept {
        state_ = state;
    }

    /**
     * @brief 获取当前计数值
     * @return 计数值
     */
    uint32_t count() const noexcept {
        return count_;
    }

private:
    /**
     * @brief 检查是否达到阈值
     *
     * 根据当前计数值更新执行状态
     */
    void CheckThreshold() noexcept {
        if (state_ == ExecutionState::kInterpreted &&
            count_ >= kBaselineThreshold) {
            // 达到Baseline编译阈值
            state_ = ExecutionState::kWarmup;
        } else if (state_ == ExecutionState::kBaseline &&
                   count_ >= kOptimizedThreshold) {
            // 达到Optimized编译阈值（预留）
            state_ = ExecutionState::kHot;
        }
    }

    uint32_t count_;              ///< 执行次数计数
    ExecutionState state_;        ///< 当前执行状态
};

} // namespace mjs::jit

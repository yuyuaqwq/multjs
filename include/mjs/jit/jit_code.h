/**
 * @file jit_code.h
 * @brief JIT代码封装类
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了JIT代码封装类，用于管理编译后的机器码
 */

#pragma once

#include <cstdint>
#include <memory>

namespace mjs {

class StackFrame;
class Value;

namespace jit {

/**
 * @class JitCode
 * @brief JIT代码封装类
 *
 * 管理编译后的机器码内存，提供执行接口
 */
class JitCode {
public:
    /**
     * @brief 构造函数
     * @param code_ptr 代码指针（指向可执行内存）
     * @param code_size 代码大小（字节）
     *
     * @note code_ptr必须是通过asmjit分配的可执行内存
     */
    JitCode(void* code_ptr, size_t code_size) noexcept;

    /**
     * @brief 析构函数
     *
     * 释放代码内存
     */
    ~JitCode();

    // 禁止拷贝
    JitCode(const JitCode&) = delete;
    JitCode& operator=(const JitCode&) = delete;

    // 允许移动
    JitCode(JitCode&& other) noexcept;
    JitCode& operator=(JitCode&& other) noexcept;

    /**
     * @brief 执行JIT代码
     * @param stack_frame 栈帧指针
     * @return 函数返回值
     *
     * @note 这是一个函数指针调用，栈帧和参数需要按照约定设置
     */
    Value Execute(StackFrame* stack_frame);

    /**
     * @brief 获取代码指针
     * @return 代码指针
     */
    void* code_ptr() const noexcept {
        return code_ptr_;
    }

    /**
     * @brief 获取代码大小
     * @return 代码大小（字节）
     */
    size_t code_size() const noexcept {
        return code_size_;
    }

    /**
     * @brief 检查代码是否有效
     * @return 是否有效
     */
    bool is_valid() const noexcept {
        return code_ptr_ != nullptr;
    }

private:
    void* code_ptr_;      ///< 代码指针（指向可执行内存）
    size_t code_size_;    ///< 代码大小（字节）
};

} // namespace jit
} // namespace mjs

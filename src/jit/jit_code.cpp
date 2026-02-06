/**
 * @file jit_code.cpp
 * @brief JIT代码封装类实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <mjs/jit/jit_code.h>
#include <mjs/stack_frame.h>
#include <mjs/value/value.h>

#ifdef ENABLE_JIT

#include <asmjit/asmjit.h>

namespace mjs::jit {

JitCode::JitCode(void* code_ptr, size_t code_size) noexcept
    : code_ptr_(code_ptr), code_size_(code_size) {}

JitCode::~JitCode() {
    // 不需要手动释放代码内存
    // JitRuntime 在析构时会自动释放所有分配的内存
}

JitCode::JitCode(JitCode&& other) noexcept
    : code_ptr_(other.code_ptr_), code_size_(other.code_size_) {
    other.code_ptr_ = nullptr;
    other.code_size_ = 0;
}

JitCode& JitCode::operator=(JitCode&& other) noexcept {
    if (this != &other) {
        // 不需要手动释放代码内存
        // JitRuntime 在析构时会自动释放所有分配的内存

        // 转移所有权
        code_ptr_ = other.code_ptr_;
        code_size_ = other.code_size_;

        other.code_ptr_ = nullptr;
        other.code_size_ = 0;
    }
    return *this;
}

Value JitCode::Execute(StackFrame* stack_frame) {
    // 定义JIT函数签名
    using JitFunction = Value (*)(StackFrame*);

    // 类型转换并调用
    auto jit_func = reinterpret_cast<JitFunction>(code_ptr_);
    return jit_func(stack_frame);
}

} // namespace mjs::jit

#endif // ENABLE_JIT

/**
 * @file jit_forward.h
 * @brief JIT前向声明
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件提供JIT相关类型的前向声明，避免在公共头文件中包含asmjit
 */

#pragma once

#ifdef ENABLE_JIT

// 前向声明asmjit类型，避免包含asmjit头文件
namespace asmjit {
class JitRuntime;
namespace x86 {
class Compiler;
class Gp;
}
class Label;
class CodeHolder;
} // namespace asmjit

#endif // ENABLE_JIT

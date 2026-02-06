/**
 * @file jit_manager.cpp
 * @brief JIT管理器实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <mjs/jit/jit_manager.h>
#include <mjs/jit/jit_code.h>
#include <mjs/jit/baseline_compiler.h>
#include <mjs/context.h>
#include <mjs/value/function_def.h>

#include <algorithm>
#include <iostream>

namespace mjs::jit {

JITManager::JITManager(Context* context)
    : context_(context), total_cache_size_(0), lru_counter_(0) {}

JITManager::~JITManager() {
    // JitCode会自动释放，unique_ptr会清理内存
}

JitCode* JITManager::GetBaselineCode(FunctionDefBase* func_def) {
    auto it = code_cache_.find(func_def);
    if (it != code_cache_.end()) {
        // 更新LRU时间戳
        lru_timestamps_[func_def] = ++lru_counter_;
        return it->second.get();
    }
    return nullptr;
}

void JITManager::PruneCache(size_t max_size) {
    if (total_cache_size_ <= max_size) {
        return;
    }

    // 按LRU时间戳排序，删除最久未使用的代码
    std::vector<std::pair<uint64_t, FunctionDefBase*>> lru_list;
    for (auto& [func_def, timestamp] : lru_timestamps_) {
        lru_list.push_back({timestamp, func_def});
    }

    std::sort(lru_list.begin(), lru_list.end());

    // 删除最老的代码，直到缓存大小满足要求
    for (auto& [timestamp, func_def] : lru_list) {
        if (total_cache_size_ <= max_size) {
            break;
        }

        auto code_it = code_cache_.find(func_def);
        if (code_it != code_cache_.end()) {
            total_cache_size_ -= code_it->second->code_size();
            code_cache_.erase(code_it);
            lru_timestamps_.erase(func_def);

            std::cout << "[JIT] Pruned function: " << func_def->name()
                      << std::endl;
        }
    }
}

#ifdef ENABLE_JIT

#include <mjs/jit/baseline_compiler.h>

void JITManager::CompileBaseline(FunctionDefBase* func_def) {
    if (!func_def) {
        return;
    }

    // 检查是否已经编译过
    if (code_cache_.find(func_def) != code_cache_.end()) {
        // 更新LRU时间戳
        lru_timestamps_[func_def] = ++lru_counter_;
        return;
    }

    // 执行编译
    auto jit_code = CompileBaselineImpl(func_def);

    if (jit_code && jit_code->is_valid()) {
        // 编译成功，添加到缓存
        size_t code_size = jit_code->code_size();
        code_cache_[func_def] = std::move(jit_code);
        lru_timestamps_[func_def] = ++lru_counter_;
        total_cache_size_ += code_size;

        std::cout << "[JIT] Compiled function: " << func_def->name()
                  << " (size: " << code_size << " bytes)" << std::endl;
    } else {
        std::cerr << "[JIT] Failed to compile function: " << func_def->name()
                  << std::endl;
    }
}

std::unique_ptr<JitCode> JITManager::CompileBaselineImpl(FunctionDefBase* func_def) {
    // 创建Baseline编译器
    BaselineCompiler compiler(context_);

    // 编译函数
    void* code_ptr = compiler.Compile(func_def);
    if (!code_ptr) {
        return nullptr;
    }

    // 创建JitCode对象
    auto jit_code = std::make_unique<JitCode>(code_ptr, 0);

    return jit_code;
}

#endif // ENABLE_JIT

} // namespace mjs::jit

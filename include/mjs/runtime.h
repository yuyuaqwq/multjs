#pragma once

#include <filesystem>

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

#include <mjs/class_def/class_def.h>

namespace mjs {

// 常量池、字节码、栈等共享资源位于Runtime
class Runtime : public noncopyable {
public:
	// using LoadModuleFunction = Value(*)(Context* context, const char* path);
	using LoadModuleFunction = std::function<Value(Context* context, const char* path)>;

public:
	Runtime();

	void set_load_module(LoadModuleFunction load_module) {
		load_module_ = load_module;
	}

	const auto& load_module() const {
		return load_module_;
	}

	void set_load_module_async(LoadModuleFunction load_module) {
		load_module_async_ = load_module;
	}

	const auto& load_module_async() const {
		return load_module_async_;
	}

	const auto& const_pool() const { return const_pool_; }
	auto& const_pool() { return const_pool_; }
	auto& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

	const auto& class_def_table() const { return class_def_table_; }
	auto& class_def_table() { return class_def_table_; }

	auto& module_cache() { return module_cache_; }

private:
	GlobalConstPool const_pool_;
	ClassDefTable class_def_table_;

	LoadModuleFunction load_module_;
	LoadModuleFunction load_module_async_;

	std::unordered_map<std::filesystem::path, Value> module_cache_;
};

} // namespace mjs
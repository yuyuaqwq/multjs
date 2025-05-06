#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>
#include <mjs/class_def_table.h>
#include <mjs/module_mgr.h>
#include <mjs/property_map.h>

namespace mjs {

// 常量池、字节码、栈等共享资源位于Runtime
class Runtime : public noncopyable {
public:
	Runtime();

	const auto& const_pool() const { return const_pool_; }
	auto& const_pool() { return const_pool_; }

	auto& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

	const auto& class_def_table() const { return class_def_table_; }
	auto& class_def_table() { return class_def_table_; }

	auto& module_mgr() { return module_mgr_; }

	auto& global_this() { return global_this_; }

private:
	GlobalConstPool const_pool_;
	ClassDefTable class_def_table_;
	ModuleMgr module_mgr_;

	PropertyMap global_this_;
};

} // namespace mjs
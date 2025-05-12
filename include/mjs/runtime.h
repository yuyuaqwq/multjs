#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>
#include <mjs/class_def_table.h>
#include <mjs/module_mgr.h>
#include <mjs/value.h>
#include <mjs/shape.h>

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

	auto& shape_manager() { return shape_manager_; }

	auto& global_this() { return global_this_; }

	const auto& class_def_table() const { return class_def_table_; }
	auto& class_def_table() { return class_def_table_; }

	auto& module_mgr() { return module_mgr_; }

private:
	GlobalConstPool const_pool_;
	ShapeManager shape_manager_;
	Value global_this_;
	ClassDefTable class_def_table_;
	ModuleMgr module_mgr_;
};

} // namespace mjs
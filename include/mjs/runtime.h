#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>
#include <mjs/class_def_table.h>
#include <mjs/module_manager.h>
#include <mjs/value.h>
#include <mjs/shape.h>
#include <mjs/gc_manager.h>

namespace mjs {

// 常量池、字节码、栈等共享资源位于Runtime
class Runtime : public noncopyable {
public:
	Runtime();
	~Runtime();

	const auto& const_pool() const { return const_pool_; }
	auto& const_pool() { return const_pool_; }

	auto& gc_manager() { return gc_manager_; }

	auto& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

	auto& shape_manager() { return shape_manager_; }

	auto& global_this() { return global_this_; }

	const auto& class_def_table() const { return class_def_table_; }
	auto& class_def_table() { return class_def_table_; }

	auto& module_manager() { return module_manager_; }

private:
	GlobalConstPool const_pool_;
	GCManager gc_manager_;
	ShapeManager shape_manager_;
	Value global_this_;
	ClassDefTable class_def_table_;
	ModuleManager module_manager_;
};

} // namespace mjs
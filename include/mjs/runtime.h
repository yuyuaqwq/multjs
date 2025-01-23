#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

namespace mjs {

// 常量池、字节码等不可变共享资源位于Runtime
class Runtime : noncopyable {
public:
	const ConstPool& const_pool() const { return const_pool_; }
	ConstPool& const_pool() { return const_pool_; }

	Stack& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

private:
	ConstPool const_pool_;
};

} // namespace mjs
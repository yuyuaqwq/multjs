#pragma once

#include <memory>

#include "noncopyable.h"
#include "const_pool.h"
#include "context.h"

namespace mjs {

// 常量池、字节码等不可变共享资源位于Runtime
class Runtime : noncopyable {
public:
	const ConstPool& const_pool() const { return const_pool_; }
	ConstPool& const_pool() { return const_pool_; }

	Stack& stack() {
		static thread_local Stack stack_(1024);
		return stack_;
	}

private:
	ConstPool const_pool_;
};

} // namespace mjs
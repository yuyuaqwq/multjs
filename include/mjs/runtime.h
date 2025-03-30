#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

namespace mjs {

// �����ء��ֽ��롢ջ�ȹ�����Դλ��Runtime
class Runtime : public noncopyable {
public:
	const GlobalConstPool& const_pool() const { return const_pool_; }
	GlobalConstPool& const_pool() { return const_pool_; }

	Stack& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

private:
	GlobalConstPool const_pool_;
};

} // namespace mjs
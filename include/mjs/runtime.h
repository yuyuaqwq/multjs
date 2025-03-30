#pragma once

#include <mjs/noncopyable.h>
#include <mjs/class.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

namespace mjs {

// 常量池、字节码、栈等共享资源位于Runtime
class Runtime : public noncopyable {
public:

	void NewClass(ClassType type) {
		// 是否是注册类，在发现编译时
		//auto class_it = g_classes.find(ident);
		//if (class_it != g_classes.end()) {
		//	token.set_type(class_it->second);
		//	return token;
		//}
	}


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
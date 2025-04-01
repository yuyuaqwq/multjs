#pragma once

#include <mjs/noncopyable.h>
#include <mjs/class_def.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

namespace mjs {

// �����ء��ֽ��롢ջ�ȹ�����Դλ��Runtime
class Runtime : public noncopyable {
public:

	void NewClass(ClassId type) {
		// �Ƿ���ע���࣬�ڷ��ֱ���ʱ
		//auto class_it = g_classes.find(ident);
		//if (class_it != g_classes.end()) {
		//	token.set_type(class_it->second);
		//	return token;
		//}


	}


	const auto& const_pool() const { return const_pool_; }
	auto& const_pool() { return const_pool_; }
	auto& stack() {
		static thread_local auto stack = Stack(1024);
		return stack;
	}

	const auto& class_def_table() const { return class_def_table_; }
	auto& class_def_table() { return class_def_table_; }

private:
	GlobalConstPool const_pool_;
	ClassDefTable class_def_table_;
};

} // namespace mjs
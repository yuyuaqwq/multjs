#pragma once

#include <mjs/noncopyable.h>
#include <mjs/const_pool.h>
#include <mjs/stack_frame.h>

#include <mjs/class_def/class_def.h>
#include <mjs/class_def/generator_class_def.h>
#include <mjs/class_def/promise_class_def.h>

namespace mjs {

// 常量池、字节码、栈等共享资源位于Runtime
class Runtime : public noncopyable {
public:
	Runtime() {
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kBase, "Object"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kNumber, "Number"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kString, "String"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kArray, "Array"));
		class_def_table_.Register(std::make_unique<GeneratorClassDef>());
		class_def_table_.Register(std::make_unique<PromiseClassDef>());

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
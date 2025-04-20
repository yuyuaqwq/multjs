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
	// using LoadModuleFunction = Value(*)(Context* context, const char* path);
	using LoadModuleFunction = std::function<Value(Context* context, const char* path)>;

public:
	Runtime() {
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kBase, "Object"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kNumber, "Number"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kString, "String"));
		class_def_table_.Register(std::make_unique<ClassDef>(ClassId::kArray, "Array"));
		class_def_table_.Register(std::make_unique<GeneratorClassDef>());
		class_def_table_.Register(std::make_unique<PromiseClassDef>());

		set_load_module_callback([](Context* ctx, const char* path) -> Value {
			//std::fstream file;
			//file.open(path);
			//auto content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());;
			//file.close();
			//auto module = ctx->Eval(content);
			//return module;
			return Value();
		});
	}



	void set_load_module_callback(LoadModuleFunction callback) {
		load_module_callback_ = callback;
	}

	const auto& load_module_callback() const {
		return load_module_callback_;
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

	LoadModuleFunction load_module_callback_;
};

} // namespace mjs
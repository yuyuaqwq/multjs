#pragma once

#include <iostream>
#include <memory>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/runtime.h>
#include <mjs/vm.h>
#include <mjs/job_queue.h>
#include <mjs/object.h>
#include <mjs/shape.h>
#include <mjs/gc_manager.h>

namespace mjs {

class Context : public noncopyable {
public:
	Context(Runtime* runtime);

	~Context();

	// 编译模块，返回模块定义
    Value CompileModule(std::string module_name, std::string_view script);

	// 执行模块
    Value CallModule(Value* value);

    Value Eval(std::string module_name, std::string_view script);
    Value EvalFromFile(std::string_view path);

	template<typename It>
	Value CallFunction(Value* func_val, Value this_val, It begin, It end) {
        auto stack_frame = StackFrame(&runtime().stack());
		return vm_.CallFunction(&stack_frame, *func_val, std::move(this_val), begin, end);
	}


	void ExecuteMicrotasks() {
		while (!microtask_queue_.empty()) {
			auto& task = microtask_queue_.front();
            CallFunction(&task.func(), task.this_val(), task.argv().begin(), task.argv().end());
			microtask_queue_.pop_front();
		}
	}

    void ReferenceConstValue(ConstIndex const_index);
    void DereferenceConstValue(ConstIndex const_index);
    ConstIndex FindConstOrInsertToLocal(const Value& value);
    ConstIndex FindConstOrInsertToGlobal(const Value& value);
    const Value& GetConstValue(ConstIndex const_index);

	auto& runtime() const { return *runtime_; }
    auto& local_const_pool() { return local_const_pool_; }
	const auto& microtask_queue() const { return microtask_queue_; }
	auto& microtask_queue() { return microtask_queue_; }
    // auto& symbol_table() { return symbol_table_; }
    auto& shape_manager() { return shape_manager_; }
	auto& gc_manager() { return gc_manager_; }

private:
    Runtime* runtime_;
	LocalConstPool local_const_pool_;
	VM vm_;
	JobQueue microtask_queue_;
    // PropertyMap symbol_table_;
    ShapeManager shape_manager_;
	GCManager gc_manager_;
};

} // namespace mjs

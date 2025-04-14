#pragma once

#include <memory>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/runtime.h>
#include <mjs/vm.h>
#include <mjs/object.h>
#include <mjs/job_queue.h>

namespace mjs {

class Context : public noncopyable {
public:
	Context(Runtime* runtime)
		: runtime_(runtime)
		, vm_(this) {}

	Value Eval(std::string_view script);

	template<typename It>
	Value Call(Value func_val, Value this_val, It begin, It end) {
		return vm_.CallFunction(StackFrame(&runtime_->stack()), std::move(func_val), std::move(this_val), begin, end);
	}

    void Gc() {
        // 第一趟将孩子解引用为0的挂入tmp，因为该孩子节点只被当前节点引用
        // 如果当前节点可以被回收，那么该孩子就肯定也要被回收

        // 第二趟扫的时候，再将当前链表中的节点指向的孩子挂回来，因为当前节点不是垃圾，其孩子自然也不是垃圾
        // 如果没有被挂回链表的节点，那就是垃圾了，没有被根节点自下的路径引用


        // 第一趟：将孩子引用计数为0的节点移动到临时链表
        intrusive_list<Object> tmp_list;

        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object* current = &(*it);
            ++it; // 提前递增迭代器，因为下面可能会移除当前元素

            // 检查所有子对象的引用计数
            current->ForEachChild([&tmp_list](Object& child) {
                child.WeakDereference();
                if (child.ref_count() == 0) {
                    child.unlink();
                    tmp_list.push_back(child);
                }
            });
        }

        // 第二趟：检查临时链表中的对象是否被根节点引用
        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object* current = &(*it);
            ++it; // 提前递增迭代器

            current->ForEachChild([this](Object& child) {
                if (child.ref_count() == 0) {
                    child.Reference();
                    child.unlink();
                    object_list_.push_back(child);
                }
            });
        }

        // 剩下的tmp_list中的节点就是垃圾，可以释放
        while (!tmp_list.empty()) {
            Object* obj = &tmp_list.front();
            obj->unlink();
            delete obj;
        }
    }

	void AddObject(Object* object) {
		object_list_.push_back(*object);
	}


	void ExecuteMicrotasks() {
		while (!microtask_queue_.empty()) {
			auto& task = microtask_queue_.front();
			Call(task.func(), task.this_val(), task.argv().begin(), task.argv().end());
			microtask_queue_.pop();
		}
	}

	//const Value& GetVar(VarIndex idx) {
	//	return vm_.GetVar(idx);
	//}

	auto& runtime() const { return *runtime_; }
	// LocalConstPool& const_pool() { return local_const_pool_; }

	const auto& microtask_queue() const { return microtask_queue_; }
	auto& microtask_queue() { return microtask_queue_; }

private:
	intrusive_list<Object> object_list_;

	Runtime* runtime_;
	// LocalConstPool local_const_pool_;
	Vm vm_;
	JobQueue microtask_queue_;
};

} // namespace mjs

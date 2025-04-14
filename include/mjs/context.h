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
        intrusive_list<Object> tmp_list;

        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;

            assert(!cur.gc_mark());

            cur.ForEachChild(&tmp_list, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                assert(obj.ref_count() > 0);
                obj.WeakDereference();
                if (obj.ref_count() == 0 && obj.gc_mark()) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;

            cur.set_gc_mark(true);
            if (cur.ref_count() == 0) {
                cur.unlink();
                tmp_list.push_back(cur);
            }
        }

        // 到这里看object_list_，没有被回收的就可能是漏清理的
        // 看一下Value::IsObject有没有添加对应的类型

        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;

            assert(cur.ref_count() > 0);

            cur.set_gc_mark(false);

            cur.ForEachChild(&object_list_, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                obj.Reference();
                if (obj.ref_count() == 1) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;
        }

        it = tmp_list.begin();
        while (it != tmp_list.end()) {
            Object& cur = *it;

            cur.ForEachChild(&object_list_, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                obj.Reference();
            });

            ++it;
        }



        // 剩下的tmp_list中的节点就是垃圾，可以释放
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            obj.WeakDereference();
            delete &obj;
        }
    }


	void AddObject(Object* object) {
		object_list_.push_back(*object);
	}


	void ExecuteMicrotasks() {
		while (!microtask_queue_.empty()) {
			auto& task = microtask_queue_.front();
			Call(task.func(), task.this_val(), task.argv().begin(), task.argv().end());
			microtask_queue_.pop_front();
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

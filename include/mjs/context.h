#pragma once

#include <iostream>
#include <memory>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/runtime.h>
#include <mjs/vm.h>
#include <mjs/job_queue.h>
#include <mjs/object/object.h>

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

    void PrintObjectTree() {
        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            std::cout << Value(&cur).ToString().string() << std::endl;

            cur.ForEachChild(nullptr, [](intrusive_list<Object>* list, const Value& child) {
                std::cout << "\t\t" << child.ToString().string() << std::endl;
            });

            std::cout << std::endl;

            ++it;
        }
    }

    void GC() {
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
                // 如果子对象的引用计数降为0，说明如果当前对象被释放，子对象也会被释放
                // 仅在子对象已经被标记过才添加到临时链表，因为子对象也需要遍历其子对象
                if (obj.ref_count() == 0 && obj.gc_mark()) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;

            // 完成扫描的对象，标记一下
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

            // 剩下的都是不可回收的对象，先清除一下标记
            cur.set_gc_mark(false);

            cur.ForEachChild(&object_list_, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                // 被不可回收的对象引用的子对象，当然也不能回收，挂回主链表
                obj.Reference();
                if (obj.ref_count() == 1) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;
        }

        // 这一趟是为了恢复可被回收对象的引用计数，为了维持引用计数的正确性
        // 在对象释放子对象时，子对象可以正确递减引用计数为0
        // 相对于mjs来说已经是无意义的了，因为mjs会在Value析构时自动释放对象
        // 为了避免Value引起的提前析构子对象，对象的递减引用只有在非gc标记阶段才能生效
        //it = tmp_list.begin();
        //while (it != tmp_list.end()) {
        //    Object& cur = *it;

        //    cur.ForEachChild(&object_list_, [](intrusive_list<Object>* list, const Value& child) {
        //        if (!child.IsObject()) return;
        //        auto& obj = child.object();
        //        obj.Reference();
        //    });

        //    ++it;
        //}



        // 剩下的tmp_list中的节点就是垃圾，可以释放
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            assert(obj.ref_count() == 0);
            assert(obj.gc_mark());
            // obj.WeakDereference();
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

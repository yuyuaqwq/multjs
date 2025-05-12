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

namespace mjs {

class Context : public noncopyable {
public:
	Context(Runtime* runtime)
		: runtime_(runtime)
		, vm_(this)
        , shape_manager_(this)
       /* , symbol_table_(this)*/ {}

    Value Compile(std::string module_name, std::string_view script);

    Value CallModule(Value* value);

    Value Eval(std::string module_name, std::string_view script);
    Value EvalByPath(std::string_view path);

	template<typename It>
	Value CallFunction(Value* func_val, Value this_val, It begin, It end) {
        auto stack_frame = StackFrame(&runtime().stack());

		return vm_.CallFunction(&stack_frame, *func_val, std::move(this_val), begin, end);
	}

    void PrintObjectTree() {
        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            std::cout << Value(&cur).ToString(this).string_view();
            std::cout << " ref_count:" << cur.ref_count();
            std::cout << std::endl;

            cur.GCForEachChild(this, nullptr, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                std::cout << "\t\t" << child.ToString(context).string_view();
                if (child.IsObject()) {
                    std::cout << " ref_count:" << child.object().ref_count();
                }
                std::cout << std::endl;
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

            cur.GCForEachChild(this, &tmp_list, [](Context* context, intrusive_list<Object>* list, const Value& child) {
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

            cur.GCForEachChild(this, &object_list_, [](Context* context, intrusive_list<Object>* list, const Value& child) {
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

        // 上面那一趟只恢复了剩下的object_list_的子对象的引用计数
        // 被释放对象的子对象的引用计数也要加回去
        it = tmp_list.begin();
        while (it != tmp_list.end()) {
            Object& cur = *it;

            cur.GCForEachChild(this, &object_list_, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                obj.Reference();
            });

            ++it;
        }


        // 剩下的tmp_list中的节点就是垃圾，可以释放
        // 这里因为需要手动开始释放对象，但是析构又会依据Value自动进行
        // 所以没法校验引用计数，忽略引用计数释放这些对象
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            // assert(obj.ref_count() == 0);
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
            CallFunction(&task.func(), task.this_val(), task.argv().begin(), task.argv().end());
			microtask_queue_.pop_front();
		}
	}

	auto& runtime() const { return *runtime_; }
	LocalConstPool& const_pool() { return local_const_pool_; }
	const auto& microtask_queue() const { return microtask_queue_; }
	auto& microtask_queue() { return microtask_queue_; }
    // auto& symbol_table() { return symbol_table_; }
    auto& shape_manager() { return shape_manager_; }

private:
    Runtime* runtime_;
	intrusive_list<Object> object_list_;
	LocalConstPool local_const_pool_;
	VM vm_;
	JobQueue microtask_queue_;
    // PropertyMap symbol_table_;

    ShapeManager shape_manager_;
};

} // namespace mjs

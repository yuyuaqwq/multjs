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

    Value Compile(std::string_view script);

    Value CallModule(Value* value);

    Value Eval(std::string_view script);
    Value EvalByPath(const char* path);

	template<typename It>
	Value CallFunction(Value* func_val, Value this_val, It begin, It end) {
        auto upper_stack_frame = StackFrame(&runtime_->stack());
        // ����������һ��func_def����ô������Ҫ����Ϊfunc_obj��module_obj
        if (func_val->IsFunctionDef()) {
            vm_.InitClosure(upper_stack_frame, func_val);
        }
		return vm_.CallFunction(upper_stack_frame, *func_val, std::move(this_val), begin, end);
	}

    void PrintObjectTree() {
        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            std::cout << Value(&cur).ToString().string();
            std::cout << " ref_count:" << cur.ref_count();
            std::cout << std::endl;

            cur.ForEachChild(nullptr, [](intrusive_list<Object>* list, const Value& child) {
                std::cout << "\t\t" << child.ToString().string();
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

            cur.ForEachChild(&tmp_list, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                assert(obj.ref_count() > 0);
                obj.WeakDereference();
                // ����Ӷ�������ü�����Ϊ0��˵�������ǰ�����ͷţ��Ӷ���Ҳ�ᱻ�ͷ�
                // �����Ӷ����Ѿ�����ǹ�����ӵ���ʱ������Ϊ�Ӷ���Ҳ��Ҫ�������Ӷ���
                if (obj.ref_count() == 0 && obj.gc_mark()) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;

            // ���ɨ��Ķ��󣬱��һ��
            cur.set_gc_mark(true);
            if (cur.ref_count() == 0) {
                cur.unlink();
                tmp_list.push_back(cur);
            }
        }

        // �����￴object_list_��û�б����յľͿ�����©�����
        // ��һ��Value::IsObject��û����Ӷ�Ӧ������

        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(cur.ref_count() > 0);

            // ʣ�µĶ��ǲ��ɻ��յĶ��������һ�±��
            cur.set_gc_mark(false);

            cur.ForEachChild(&object_list_, [](intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                // �����ɻ��յĶ������õ��Ӷ��󣬵�ȻҲ���ܻ��գ��һ�������
                obj.Reference();
                if (obj.ref_count() == 1) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;
        }

        // ������һ��ֻ�ָ���ʣ�µ�object_list_���Ӷ�������ü���
        // ���ͷŶ�����Ӷ�������ü���ҲҪ�ӻ�ȥ
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


        // ʣ�µ�tmp_list�еĽڵ���������������ͷ�
        // ������Ϊ��Ҫ�ֶ���ʼ�ͷŶ��󣬵��������ֻ�����Value�Զ�����
        // ����û��У�����ü������������ü����ͷ���Щ����
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
	// LocalConstPool& const_pool() { return local_const_pool_; }

	const auto& microtask_queue() const { return microtask_queue_; }
	auto& microtask_queue() { return microtask_queue_; }

private:
    Runtime* runtime_;

	intrusive_list<Object> object_list_;

	// LocalConstPool local_const_pool_;

	Vm vm_;

	JobQueue microtask_queue_;
};

} // namespace mjs

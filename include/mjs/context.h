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

        // ��һ����Ϊ�˻ָ��ɱ����ն�������ü�����Ϊ��ά�����ü�������ȷ��
        // �ڶ����ͷ��Ӷ���ʱ���Ӷ��������ȷ�ݼ����ü���Ϊ0
        // �����mjs��˵�Ѿ�����������ˣ���Ϊmjs����Value����ʱ�Զ��ͷŶ���
        // Ϊ�˱���Value�������ǰ�����Ӷ��󣬶���ĵݼ�����ֻ���ڷ�gc��ǽ׶β�����Ч
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



        // ʣ�µ�tmp_list�еĽڵ���������������ͷ�
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            assert(obj.ref_count() == 0);
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

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
        // ��һ�˽����ӽ�����Ϊ0�Ĺ���tmp����Ϊ�ú��ӽڵ�ֻ����ǰ�ڵ�����
        // �����ǰ�ڵ���Ա����գ���ô�ú��ӾͿ϶�ҲҪ������

        // �ڶ���ɨ��ʱ���ٽ���ǰ�����еĽڵ�ָ��ĺ��ӹһ�������Ϊ��ǰ�ڵ㲻���������亢����ȻҲ��������
        // ���û�б��һ�����Ľڵ㣬�Ǿ��������ˣ�û�б����ڵ����µ�·������


        // ��һ�ˣ����������ü���Ϊ0�Ľڵ��ƶ�����ʱ����
        intrusive_list<Object> tmp_list;

        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object* current = &(*it);
            ++it; // ��ǰ��������������Ϊ������ܻ��Ƴ���ǰԪ��

            // ��������Ӷ�������ü���
            current->ForEachChild([&tmp_list](Object& child) {
                child.WeakDereference();
                if (child.ref_count() == 0) {
                    child.unlink();
                    tmp_list.push_back(child);
                }
            });
        }

        // �ڶ��ˣ������ʱ�����еĶ����Ƿ񱻸��ڵ�����
        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object* current = &(*it);
            ++it; // ��ǰ����������

            current->ForEachChild([this](Object& child) {
                if (child.ref_count() == 0) {
                    child.Reference();
                    child.unlink();
                    object_list_.push_back(child);
                }
            });
        }

        // ʣ�µ�tmp_list�еĽڵ���������������ͷ�
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

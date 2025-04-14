#pragma once

#include <memory>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/runtime.h>
#include <mjs/vm.h>
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
	Object* obj_list_head_prev_;
	Object* obj_list_head_next_;

	Runtime* runtime_;
	// LocalConstPool local_const_pool_;
	Vm vm_;
	JobQueue microtask_queue_;
};

} // namespace mjs

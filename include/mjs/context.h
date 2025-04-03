#pragma once

#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/vm.h>
#include <mjs/job_queue.h>

namespace mjs {

class Runtime;
class Context : public noncopyable {
public:
	Context(Runtime* runtime)
		: runtime_(runtime)
		, vm_(this) {}

	Value Eval(std::string_view script);

	Value Call(Value func, Value this_val, const std::vector<Value>& argv);

	void Gc() {
		// ��һ�˽����ӽ�����Ϊ0�Ĺ���tmp����Ϊ�ú��ӽڵ�ֻ����ǰ�ڵ�����
		// �����ǰ�ڵ���Ա����գ���ô�ú��ӾͿ϶�ҲҪ������

		// �ڶ���ɨ��ʱ���ٽ���ǰ�����еĽڵ�ָ��ĺ��ӹһ�������Ϊ��ǰ�ڵ㲻���������亢����ȻҲ��������
		// ���û�б��һ�����Ľڵ㣬�Ǿ��������ˣ�û�б����ڵ����µ�·������
	}

	void ExecuteMicrotasks() {
		while (!microtask_queue_.empty()) {
			auto& task = microtask_queue_.front();
			Call(task.func(), task.this_val(), task.argv());
			microtask_queue_.pop();
		}
	}

	const Value& GetVar(VarIndex idx) {
		return vm_.GetVar(idx);
	}

	auto& runtime() const { return *runtime_; }
	// LocalConstPool& const_pool() { return local_const_pool_; }

	const auto& microtask_queue() const { return microtask_queue_; }
	auto& microtask_queue() { return microtask_queue_; }

private:
	Runtime* runtime_;
	// LocalConstPool local_const_pool_;
	Vm vm_;
	JobQueue microtask_queue_;
};

} // namespace mjs
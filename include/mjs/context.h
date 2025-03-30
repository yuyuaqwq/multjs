#pragma once

#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/vm.h>

namespace mjs {

class Runtime;
class Context : public noncopyable {
public:
	Context(Runtime* runtime)
		: runtime_(runtime)
		, vm_(this) {}

	void Eval(std::string_view script);

	void Gc() {
		// ��һ�˽����ӽ�����Ϊ0�Ĺ���tmp����Ϊ�ú��ӽڵ�ֻ����ǰ�ڵ�����
		// �����ǰ�ڵ���Ա����գ���ô�ú��ӾͿ϶�ҲҪ������

		// �ڶ���ɨ��ʱ���ٽ���ǰ�����еĽڵ�ָ��ĺ��ӹһ�������Ϊ��ǰ�ڵ㲻���������亢����ȻҲ��������
		// ���û�б��һ�����Ľڵ㣬�Ǿ��������ˣ�û�б����ڵ����µ�·������
	}

	Runtime& runtime() const { return *runtime_; }
	// LocalConstPool& const_pool() { return local_const_pool_; }

private:
	Runtime* runtime_;

	// LocalConstPool local_const_pool_;

	Vm vm_;
};

} // namespace mjs
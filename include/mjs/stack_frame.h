#pragma once

#include <vector>
#include <string>
#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

// StackFrame��
// ʹ��tls��Ϊÿ���߳�ָ��һ���ڴ�(��1M)
// ��ǰStackFrame��������ֱ������������ڴ���
// ����ʱ����Ϊ�ɵ�StackFrame��������
// �µ�StackFrame��ָ�룬ֱ��ָ����ڴ��δʹ�ò���(�����Ǿɵ�StackFrame)��Ȼ��ʹ���µ�StackFrame����
class Stack;
class StackFrame {
public:
	StackFrame(Stack* stack)
		: stack_(stack){}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	// ������ʾ��ջ֡������������0��ʼ
	// ������ʾ��ջ֡������������-1��ʼ
	Value& Get(ptrdiff_t index);
	void Set(ptrdiff_t index, const Value& value);
	void Set(ptrdiff_t index, Value&& value);

	size_t bottom() const { return bottom_; }
	void set_bottom(size_t bottom) { bottom_ = bottom; }

	const auto& this_val() const { return this_val_; }
	void set_this_val(Value&& this_val) { this_val_ = this_val; }

private:
	Stack* stack_;
	Value this_val_;
	size_t bottom_ = 0;	// ��ǰջ֡��ջ��(��ջ�е�����)
};

// ÿ���̶̹߳���ջ
class Stack : noncopyable {
public:
	Stack(size_t count) {
		vector_.reserve(count);
	}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	Value& Get(size_t index);
	void Set(size_t index, const Value& value);
	void Set(size_t index, Value&& value);

	void Upgrade(size_t size);
	void Reduce(size_t size);

	size_t Size()  const noexcept;
	void Resize(size_t size);

	auto& vector() { return vector_; }

private:
	std::vector<Value> vector_;
};


} // namespace mjs
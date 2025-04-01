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
class StackFrame : public noncopyable {
public:
	StackFrame(Stack* stack)
		: stack_(stack){}

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	void reduce(size_t count);

	// ������ʾ��ջ֡������������0��ʼ
	// ������ʾ��ջ֡������������-1��ʼ
	Value& get(ptrdiff_t index);
	void set(ptrdiff_t index, const Value& value);
	void set(ptrdiff_t index, Value&& value);

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
class Stack : public noncopyable {
public:
	Stack(size_t count) {
		vector_.reserve(count);
	}

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	Value& get(size_t index);
	void set(size_t index, const Value& value);
	void set(size_t index, Value&& value);

	void upgrade(size_t size);
	void reduce(size_t size);

	size_t size()  const noexcept;
	void resize(size_t size);

	auto& vector() { return vector_; }

private:
	std::vector<Value> vector_;
};


} // namespace mjs
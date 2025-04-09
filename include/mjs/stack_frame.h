#pragma once

#include <vector>
#include <string>
#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/opcode.h>

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

	StackFrame(const StackFrame& upper_stack_frame);

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	void reduce(size_t count);
	void upgrade(size_t count);

	// ������ʾ��ջ֡������������0��ʼ
	// ������ʾ��ջ֡������������-1��ʼ
	Value& get(ptrdiff_t index) const;
	void set(ptrdiff_t index, const Value& value);
	void set(ptrdiff_t index, Value&& value);

	size_t bottom() const { return bottom_; }
	void set_bottom(size_t bottom) { bottom_ = bottom; }

	const auto& func_val() const { return func_val_; }
	void set_func_val(Value&& func_val) { func_val_ = std::move(func_val); }

	const auto* func_def() const { return func_def_; }
	void set_func_def(FunctionDef* func_def) { func_def_ = func_def; }

	const auto& this_val() const { return this_val_; }
	void set_this_val(Value&& this_val) { this_val_ = std::move(this_val); }

	auto pc() const { return pc_; }
	void set_pc(Pc pc) { pc_ = pc; }

private:
	Stack* stack_;
	size_t bottom_ = 0;	// ��ǰջ֡��ջ��(��ջ�е�����)

	Value func_val_;
	FunctionDef* func_def_ = nullptr;
	Value this_val_;
	Pc pc_ = 0;
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

inline StackFrame::StackFrame(const StackFrame& upper_stack_frame) {
	stack_ = upper_stack_frame.stack_;
	bottom_ = upper_stack_frame.stack_->size();
}

} // namespace mjs
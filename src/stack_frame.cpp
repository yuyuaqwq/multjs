#include <mjs/stack_frame.h>

namespace mjs {

void Stack::Push(const Value& value) {
	vector_.emplace_back(value);
}

void Stack::Push(Value&& value) {
	vector_.emplace_back(std::move(value));
}

Value Stack::Pop() {
	auto value = std::move(vector_.back());
	vector_.pop_back();
	return value;
}

Value& Stack::Get(size_t index) {
	return vector_[index];
}

void Stack::Set(size_t index, const Value& value) {
	vector_[index] = value;
}

void Stack::Set(size_t index, Value&& value) {
	vector_[index] = std::move(value);
}

void Stack::Upgrade(size_t size) {
	vector_.resize(vector_.size() + size);
}

void Stack::Reduce(size_t size) {
	vector_.resize(vector_.size() - size);
}


size_t Stack::Size() const noexcept {
	return vector_.size();
}

void Stack::Resize(size_t size) {
	vector_.resize(size);
}



void StackFrame::Push(const Value& value) {
	stack_->Push(value);
}

void StackFrame::Push(Value&& value) {
	stack_->Push(std::move(value));
}

Value StackFrame::Pop() {
	return stack_->Pop();
}

// ������ʾ��ջ����������
Value& StackFrame::Get(ptrdiff_t index) {
	if (index >= 0) {
		return stack_->Get(bottom_ + index);
	}
	else {
		return stack_->Get(stack_->Size() + index);
	}
}

void StackFrame::Set(ptrdiff_t index, const Value& value) {
	auto value_ = value;
	Set(index, std::move(value_));
}

void StackFrame::Set(ptrdiff_t index, Value&& value) {
	if (index >= 0) {
		stack_->Set(bottom_ + index, value);
	}
	else {
		stack_->Set(stack_->Size() + index, value);
	}
}

} // namespace mjs
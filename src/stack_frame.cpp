#include <mjs/stack_frame.h>

namespace mjs {

void Stack::Push(const Value& value) {
	stack_.emplace_back(value);
}

void Stack::Push(Value&& value) {
	stack_.emplace_back(std::move(value));
}

Value Stack::Pop() {
	auto value = std::move(stack_.back());
	stack_.pop_back();
	return value;
}

Value& Stack::Get(size_t index) {
	return stack_[index];
}

void Stack::Set(size_t index, const Value& value) {
	stack_[index] = value;
}

void Stack::Set(size_t index, Value&& value) {
	stack_[index] = std::move(value);
}

void Stack::Upgrade(size_t size) {
	stack_.resize(stack_.size() + size);
}

void Stack::Reduce(size_t size) {
	stack_.resize(stack_.size() - size);
}


size_t Stack::Size() const noexcept {
	return stack_.size();
}

void Stack::Resize(size_t size) {
	stack_.resize(size);
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

// 负数表示从栈顶向下索引
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
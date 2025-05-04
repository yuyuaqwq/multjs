#include <mjs/stack_frame.h>

namespace mjs {

void Stack::push(const Value& value) {
	vector_.emplace_back(value);
}

void Stack::push(Value&& value) {
	vector_.emplace_back(std::move(value));
}

Value Stack::pop() {
	auto value = std::move(vector_.back());
	vector_.pop_back();
	return value;
}

Value& Stack::get(size_t index) {
	return vector_[index];
}

void Stack::set(size_t index, const Value& value) {
	vector_[index] = value;
}

void Stack::set(size_t index, Value&& value) {
	vector_[index] = std::move(value);
}

void Stack::upgrade(size_t size) {
	vector_.resize(vector_.size() + size);
}

void Stack::reduce(size_t size) { 
	assert(vector_.size() >= size);
	vector_.resize(vector_.size() - size);
}


size_t Stack::size() const noexcept {
	return vector_.size();
}

void Stack::resize(size_t size) {
	vector_.resize(size);
}


StackFrame::StackFrame(Stack* stack)
	: stack_(stack)
	, bottom_(stack->size()) {}

StackFrame::StackFrame(const StackFrame* upper_stack_frame)
	: upper_stack_frame_(upper_stack_frame)
{
	stack_ = upper_stack_frame->stack_;
	bottom_ = upper_stack_frame->stack_->size();
}

void StackFrame::push(const Value& value) {
	stack_->push(value);
}

void StackFrame::push(Value&& value) {
	stack_->push(std::move(value));
}

Value StackFrame::pop() {
	return stack_->pop();
}

void StackFrame::reduce(size_t count) {
	stack_->reduce(count);
}

void StackFrame::upgrade(size_t count) {
	stack_->upgrade(count);
}

// 负数表示从栈顶向下索引
Value& StackFrame::get(ptrdiff_t index) const {
	if (index >= 0) {
		return stack_->get(bottom_ + index);
	}
	else {
		return stack_->get(stack_->size() + index);
	}
}

void StackFrame::set(ptrdiff_t index, const Value& value) {
	auto value_ = value;
	set(index, std::move(value_));
}

void StackFrame::set(ptrdiff_t index, Value&& value) {
	if (index >= 0) {
		stack_->set(bottom_ + index, value);
	}
	else {
		stack_->set(stack_->size() + index, value);
	}
}

} // namespace mjs
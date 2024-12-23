#include "stack_frame.h"

#include "value.h"

namespace mjs {

void StackFrame::Push(const Value& value) {
	stack_.emplace_back(value);
}

Value StackFrame::Pop() {
	auto value = std::move(stack_.back());
	stack_.pop_back();
	return value;
}

// 负数表示从尾部索引起
Value& StackFrame::Get(int32_t index) {
	if (index >= 0) {
		return stack_[index];
	}
	else {
		return stack_[stack_.size() + index];
	}
}

void StackFrame::Set(int32_t index, const Value& value) {
	if (index >= 0) {
		stack_[index] = value;
	}
	else {
		stack_[stack_.size() + index] = value;
	}
}

size_t StackFrame::Size() const noexcept {
	return stack_.size();
}

void StackFrame::ReSize(size_t s) {
	return stack_.resize(s);
}

} // namespace mjs
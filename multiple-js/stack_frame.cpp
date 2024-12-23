#include "stack_frame.h"

#include "value.h"

namespace mjs {

//void StackFrame::Push(std::unique_ptr<Value>&& value) {
//	container_.push_back(std::move(value));
//}
//
//std::unique_ptr<Value> StackFrame::Pop() {
//	auto value = std::move(container_[container_.size() - 1]);
//	container_.pop_back();
//	return value;
//}
//
//// 负数表示从尾部索引起
//std::unique_ptr<Value>& StackFrame::Get(int32_t index) {
//	if (index >= 0) {
//		return container_[index];
//	}
//	else {
//		return container_[container_.size() + index];
//	}
//}
//
//void StackFrame::Set(int32_t index, std::unique_ptr<Value> value) {
//	Get(index) = std::move(value);
//}
//
//size_t StackFrame::Size() const noexcept {
//	return container_.size();
//}
//
//void StackFrame::ReSize(size_t size) {
//	return container_.resize(size);
//}
//
//void StackFrame::Clear() noexcept {
//	container_.clear();
//}


} // namespace mjs
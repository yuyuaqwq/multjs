#include "section.h"

#include "value.h"

namespace mjs {

void ValueSection::Push(std::unique_ptr<Value>&& value) {
	container_.push_back(std::move(value));
}

std::unique_ptr<Value> ValueSection::Pop() {
	auto value = std::move(container_[container_.size() - 1]);
	container_.pop_back();
	return value;
}

// 负数表示从尾部索引起
std::unique_ptr<Value>& ValueSection::Get(int32_t index) {
	if (index >= 0) {
		return container_[index];
	}
	else {
		return container_[container_.size() + index];
	}
}

void ValueSection::Set(int32_t index, std::unique_ptr<Value> value) {
	Get(index) = std::move(value);
}

size_t ValueSection::Size() const noexcept {
	return container_.size();
}

void ValueSection::ReSize(size_t size) {
	return container_.resize(size);
}

void ValueSection::Clear() noexcept {
	container_.clear();
}

void ValueSection::operator=(ValueSection&& vs) noexcept {
	container_ = std::move(vs.container_);
}

} // namespace mjs
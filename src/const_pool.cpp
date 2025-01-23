#include "const_pool.h"

namespace mjs {

uint32_t ConstPool::New(const Value& value) {
	auto value_ = value;
	return New(std::move(value_));
}

uint32_t ConstPool::New(Value&& value) {
	uint32_t const_idx;
	auto it = const_map_.find(value);
	if (it == const_map_.end()) {
		const_idx = pool_.size();
		pool_.emplace_back(std::move(value));
		const_map_.emplace(value, const_idx);
	}
	else {
		const_idx = it->second;
	}
	return const_idx;
}

// 负数表示从尾部索引起
const Value& ConstPool::Get(int32_t index) const {
	if (index >= 0) {
		return pool_[index];
	}
	else {
		return pool_[pool_.size() + index];
	}
}

size_t ConstPool::Size() const noexcept {
	return pool_.size();
}

} // namespace mjs
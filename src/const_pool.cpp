#include <mjs/const_pool.h>

namespace mjs {

ConstIndex GlobalConstPool::New(const Value& value) {
	auto value_ = value;
	return New(std::move(value_));
}

ConstIndex GlobalConstPool::New(Value&& value) {
	auto lock = std::lock_guard(mutex_);
	auto it = const_map_.find(value);
	if (it != const_map_.end()) {
		return it->second;
	}
	if (const_index_ % kStaticArraySize == 0) {
		auto i1 = const_index_ / kStaticArraySize;
		if (i1 >= kStaticArraySize) {
			throw std::overflow_error("The number of constants exceeds the upper limit.");
		}
		pool_[i1] = std::make_unique<StaticArray>();
	}
	auto const_idx = const_index_++;
	const_map_.emplace(value, const_idx);
	Get(const_idx) = std::move(value);
	return const_idx;
}

const Value& GlobalConstPool::Get(ConstIndex index) const {
	return const_cast<GlobalConstPool*>(this)->Get(index);
}

Value& GlobalConstPool::Get(ConstIndex index) {
	auto i1 = index / kStaticArraySize;
	auto i2 = index % kStaticArraySize;
	return (*pool_[i1])[i2];
}

} // namespace mjs
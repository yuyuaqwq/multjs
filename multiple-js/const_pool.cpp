#include "const_pool.h"

namespace mjs {

void ConstPool::Push(const Value& value) {
	pool_.emplace_back(value);
}

// 负数表示从尾部索引起
Value& ConstPool::Get(int32_t index) {
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
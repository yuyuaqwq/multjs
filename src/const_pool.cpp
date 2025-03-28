#include <mjs/const_pool.h>

namespace mjs {

GlobalConstPool::GlobalConstPool() {
	pool_[0] = std::make_unique<StaticArray>();
}

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
	auto const_idx = ConstToGlobalIndex(const_index_++);
	auto& val = Get(const_idx);
	val = std::move(value);
	val.set_const_index(const_idx);

	auto res = const_map_.emplace(val, const_idx);
	return const_idx;
}

const Value& GlobalConstPool::Get(ConstIndex index) const {
	return const_cast<GlobalConstPool*>(this)->Get(index);
}

Value& GlobalConstPool::Get(ConstIndex index) {
	index = GlobalToConstIndex(index);
	auto i1 = index / kStaticArraySize;
	auto i2 = index % kStaticArraySize;
	return (*pool_[i1])[i2];
}

std::optional<ConstIndex> GlobalConstPool::Find(const Value& value) {
	auto lock = std::lock_guard(mutex_);
	auto it = const_map_.find(value);
	if (it != const_map_.end()) {
		return it->second;
	}
	return std::nullopt;
}


LocalConstPool::LocalConstPool() {
	pool_.resize(1);
}

ConstIndex LocalConstPool::New(const Value& value) {
	auto value_ = value;
	return New(std::move(value_));
}

ConstIndex LocalConstPool::New(Value&& value) {
	auto it = const_map_.find(value);
	if (it != const_map_.end()) {
		return it->second;
	}
	auto const_idx = ConstToLocalIndex(pool_.size());
	auto res = const_map_.emplace(std::move(value), const_idx);
	pool_.emplace_back(const_cast<Value*>(&res.first->first));
	pool_.back()->set_const_index(const_idx);
	return const_idx;
}

const Value& LocalConstPool::Get(ConstIndex index) const {
	return const_cast<LocalConstPool*>(this)->Get(index);
}

Value& LocalConstPool::Get(ConstIndex index) {
	index = LocalToConstIndex(index);
	return *pool_[index];
}


} // namespace mjs
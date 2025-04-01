#include <mjs/const_pool.h>

namespace mjs {

LocalConstPool::LocalConstPool() {
	pool_.resize(1);
}

ConstIndex LocalConstPool::insert(const Value& value) {
	auto value_ = value;
	return insert(std::move(value_));
}

ConstIndex LocalConstPool::insert(Value&& value) {
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

const Value& LocalConstPool::get(ConstIndex index) const {
	return const_cast<LocalConstPool*>(this)->get(index);
}

Value& LocalConstPool::get(ConstIndex index) {
	index = LocalToConstIndex(index);
	return *pool_[index];
}


} // namespace mjs
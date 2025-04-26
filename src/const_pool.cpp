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
	auto it = map_.find(value);
	if (it != map_.end()) {
		return it->second;
	}
	auto const_idx = ConstIndex(pool_.size()).to_local_index();
	auto res = map_.emplace(std::move(value), const_idx);
	if (first_ == -1) {
		pool_.emplace_back(Node{ .value_ = const_cast<Value*>(&res.first->first) });
	}
	else {
		first_ = pool_[first_].next_;
		pool_[first_] = Node{ .value_ = const_cast<Value*>(&res.first->first) };
	}
	pool_.back().value_->set_const_index(const_idx);
	return const_idx;
}

void LocalConstPool::erase(ConstIndex index) {
	auto& val = *pool_[index].value_;
	map_.erase(val);
	pool_[index].next_ = first_;
	first_ = index;
}

const Value& LocalConstPool::at(ConstIndex index) const {
	return const_cast<LocalConstPool*>(this)->at(index);
}

Value& LocalConstPool::at(ConstIndex index) {
	index.from_local_index();
	return *pool_.at(index).value_;
}


} // namespace mjs
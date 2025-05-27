#include <mjs/const_pool.h>

namespace mjs {

ConstIndex GlobalConstPool::insert(const Value& value) {
	auto value_ = value;
	return insert(std::move(value_));
}

ConstIndex GlobalConstPool::insert(Value&& value) {
	auto lock = std::lock_guard(mutex_);

	auto it = map_.find(value);
	if (it != map_.end()) {
		return it->second;
	}

	if (value.IsString()) {
		printf("[global_insert]: %s\n", value.string_view());
	}

	// �Զ���StringView����ΪString
	if (value.IsStringView()) {
		value = Value(String::New(value.string_view()));
	}

	auto idx = Base::insert(std::move(value));
	auto& val = operator[](idx);

	val.set_const_index(idx);
	auto res = map_.emplace(val, idx);

	return idx;
}

std::optional<ConstIndex> GlobalConstPool::find(const Value& value) {
	auto lock = std::lock_guard(mutex_);
	auto it = map_.find(value);
	if (it != map_.end()) {
		return it->second;
	}
	return std::nullopt;
}



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

	if (value.IsString()) {
		printf("[local_insert]: %s\n", value.string_view());
	}

	auto const_idx = pool_.size();
	auto res = map_.emplace(std::move(value), const_idx);
	if (first_ == -1) {
		pool_.emplace_back(Node{ .value_ = const_cast<Value*>(&res.first->first) });
	}
	else {
		first_ = pool_[first_].next_;
		pool_[first_] = Node{ .value_ = const_cast<Value*>(&res.first->first) };
	}
	const_idx = -const_idx;
	pool_.back().value_->set_const_index(const_idx);
	return const_idx;
}

std::optional<ConstIndex> LocalConstPool::find(const Value& value) {
	auto it = map_.find(value);
	if (it != map_.end()) {
		return -it->second;
	}
	return std::nullopt;
}

void LocalConstPool::erase(ConstIndex index) {
	index = -index;
	auto& val = *pool_[index].value_;
	map_.erase(val);
	pool_[index].next_ = first_;
	first_ = index;
}

const Value& LocalConstPool::at(ConstIndex index) const {
	return const_cast<LocalConstPool*>(this)->at(index);
}

Value& LocalConstPool::at(ConstIndex index) {
	return *pool_.at(-index).value_;
}


} // namespace mjs
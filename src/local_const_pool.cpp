#include <mjs/local_const_pool.h>

namespace mjs {

LocalConstPool::LocalConstPool() {
	pool_.resize(1);
}

ConstIndex LocalConstPool::Insert(const Value& value) {
	auto value_ = value;
	return Insert(std::move(value_));
}

ConstIndex LocalConstPool::Insert(Value&& value) {
	auto it = map_.find(value);
	if (it != map_.end()) {
		// 常量已存在
		return -it->second;  // 转换为负索引
	}

	// TODO: 调试输出
	// if (value.IsString()) {
	// 	printf("[local_insert]: %s\n", value.string_view());
	// }

	// assert(!value.IsStringView());

	// TODO: StringView 提升优化
	// 自动将StringView提升为String
	// if (value.IsStringView()) {
	// 	value = Value(String::New(value.string_view()));
	// }


	auto const_index = kConstIndexInvalid;
	if (first_ == -1) {
		const_index = pool_.size();
		auto res = map_.emplace(std::move(value), const_index);
		pool_.emplace_back(Node{ .value_ = const_cast<Value*>(&res.first->first) });
	}
	else {
		const_index = first_;
		first_ = pool_[first_].next_;
		auto res = map_.emplace(std::move(value), const_index);
		pool_[const_index] = Node{ .value_ = const_cast<Value*>(&res.first->first) };
	}
	const_index = -const_index;
	pool_.back().value_->set_const_index(const_index);
	return const_index;
}

std::optional<ConstIndex> LocalConstPool::Find(const Value& value) {
	auto it = map_.find(value);
	if (it != map_.end()) {
		return -it->second;
	}
	return std::nullopt;
}

void LocalConstPool::Erase(ConstIndex index) {
	index = -index;
	auto& val = *pool_[index].value_;
	map_.erase(val);
	pool_[index].next_ = first_;
	first_ = index;
}

const Value& LocalConstPool::At(ConstIndex index) const {
	return const_cast<LocalConstPool*>(this)->At(index);
}

Value& LocalConstPool::At(ConstIndex index) {
	return *pool_.at(-index).value_;
}

} // namespace mjs
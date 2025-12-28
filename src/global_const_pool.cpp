#include <mjs/global_const_pool.h>

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

	// TODO: 调试输出
	// if (value.IsString()) {
	// 	printf("[global_insert]: %s\n", value.string_view());
	// }

	// assert(!value.IsStringView());

	// TODO: StringView 提升优化
	// 自动将StringView提升为String，能减少hash计算开销吗？
    // 实际上没有，哈希表只有在插入的时候才会计算哈希值，已经插入的值不会再用哈希比较，直接进行值比较
	// 如果是StringView，可以直接比较地址
	// if (value.IsStringView()) {
	// 	value = Value(String::New(value.string_view()));
	// }

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

} // namespace mjs
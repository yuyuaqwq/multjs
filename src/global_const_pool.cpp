#include <mjs/global_const_pool.h>

#include <mjs/const_index_embedded.h>

namespace mjs {

void GlobalConstPool::Initialize() {
	auto index = FindOrInsert(Value("__proto__"));
	assert(index == ConstIndexEmbedded::kProto);
	index = FindOrInsert(Value("prototype"));
	assert(index == ConstIndexEmbedded::kPrototype);
	index = FindOrInsert(Value("constructor"));
	assert(index == ConstIndexEmbedded::kConstructor);

	index = FindOrInsert(Value("value"));
	assert(index == ConstIndexEmbedded::kValue);
	index = FindOrInsert(Value("done"));
	assert(index == ConstIndexEmbedded::kDone);
	index = FindOrInsert(Value("next"));
	assert(index == ConstIndexEmbedded::kNext);

	index = FindOrInsert(Value("then"));
	assert(index == ConstIndexEmbedded::kThen);
	index = FindOrInsert(Value("resolve"));
	assert(index == ConstIndexEmbedded::kResolve);
	index = FindOrInsert(Value("reject"));
	assert(index == ConstIndexEmbedded::kReject);

	index = FindOrInsert(Value("split"));
	assert(index == ConstIndexEmbedded::kSplit);
	index = FindOrInsert(Value("substring"));
	assert(index == ConstIndexEmbedded::kSubString);
	index = FindOrInsert(Value("indexoOf"));
	assert(index == ConstIndexEmbedded::kIndexOf);
	index = FindOrInsert(Value("toLowerCase"));
	assert(index == ConstIndexEmbedded::kToLowerCase);
	index = FindOrInsert(Value("toUpperCase"));
	assert(index == ConstIndexEmbedded::kToUpperCase);
	index = FindOrInsert(Value("trim"));
	assert(index == ConstIndexEmbedded::kTrim);
	index = FindOrInsert(Value("replace"));
	assert(index == ConstIndexEmbedded::kReplace);
	index = FindOrInsert(Value("for"));
	assert(index == ConstIndexEmbedded::kFor);

	index = FindOrInsert(Value("freeze"));
	assert(index == ConstIndexEmbedded::kFreeze);
	index = FindOrInsert(Value("seal"));
	assert(index == ConstIndexEmbedded::kSeal);
	index = FindOrInsert(Value("preventExtensions"));
	assert(index == ConstIndexEmbedded::kPreventExtensions);
	index = FindOrInsert(Value("defineProperty"));
	assert(index == ConstIndexEmbedded::kDefineProperty);

	index = FindOrInsert(Value("length"));
	assert(index == ConstIndexEmbedded::kLength);
	index = FindOrInsert(Value("of"));
	assert(index == ConstIndexEmbedded::kOf);
	index = FindOrInsert(Value("push"));
	assert(index == ConstIndexEmbedded::kPush);
	index = FindOrInsert(Value("pop"));
	assert(index == ConstIndexEmbedded::kPop);
	index = FindOrInsert(Value("forEach"));
	assert(index == ConstIndexEmbedded::kForEach);
	index = FindOrInsert(Value("map"));
	assert(index == ConstIndexEmbedded::kMap);
	index = FindOrInsert(Value("filter"));
	assert(index == ConstIndexEmbedded::kFilter);
	index = FindOrInsert(Value("reduce"));
	assert(index == ConstIndexEmbedded::kReduce);

	assert(size() == ConstIndexEmbedded::kEnd);
}

ConstIndex GlobalConstPool::FindOrInsert(const Value& value) {
	auto value_ = value;
	return FindOrInsert(std::move(value_));
}

ConstIndex GlobalConstPool::FindOrInsert(Value&& value) {
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

std::optional<ConstIndex> GlobalConstPool::Find(const Value& value) {
	auto lock = std::lock_guard(mutex_);
	auto it = map_.find(value);
	if (it != map_.end()) {
		return it->second;
	}
	return std::nullopt;
}

} // namespace mjs
#include <mjs/global_const_pool.h>

#include <mjs/const_index_embedded.h>

namespace mjs {

void GlobalConstPool::Initialize() {
	auto index = Insert(Value("__proto__"));
	assert(index == ConstIndexEmbedded::kProto);
	index = Insert(Value("prototype"));
	assert(index == ConstIndexEmbedded::kPrototype);
	index = Insert(Value("constructor"));
	assert(index == ConstIndexEmbedded::kConstructor);

	index = Insert(Value("value"));
	assert(index == ConstIndexEmbedded::kValue);
	index = Insert(Value("done"));
	assert(index == ConstIndexEmbedded::kDone);
	index = Insert(Value("next"));
	assert(index == ConstIndexEmbedded::kNext);

	index = Insert(Value("then"));
	assert(index == ConstIndexEmbedded::kThen);
	index = Insert(Value("resolve"));
	assert(index == ConstIndexEmbedded::kResolve);
	index = Insert(Value("reject"));
	assert(index == ConstIndexEmbedded::kReject);

	index = Insert(Value("split"));
	assert(index == ConstIndexEmbedded::kSplit);
	index = Insert(Value("substring"));
	assert(index == ConstIndexEmbedded::kSubString);
	index = Insert(Value("indexoOf"));
	assert(index == ConstIndexEmbedded::kIndexOf);
	index = Insert(Value("toLowerCase"));
	assert(index == ConstIndexEmbedded::kToLowerCase);
	index = Insert(Value("toUpperCase"));
	assert(index == ConstIndexEmbedded::kToUpperCase);
	index = Insert(Value("trim"));
	assert(index == ConstIndexEmbedded::kTrim);
	index = Insert(Value("replace"));
	assert(index == ConstIndexEmbedded::kReplace);
	index = Insert(Value("for"));
	assert(index == ConstIndexEmbedded::kFor);

	index = Insert(Value("freeze"));
	assert(index == ConstIndexEmbedded::kFreeze);
	index = Insert(Value("seal"));
	assert(index == ConstIndexEmbedded::kSeal);
	index = Insert(Value("preventExtensions"));
	assert(index == ConstIndexEmbedded::kPreventExtensions);
	index = Insert(Value("defineProperty"));
	assert(index == ConstIndexEmbedded::kDefineProperty);

	index = Insert(Value("length"));
	assert(index == ConstIndexEmbedded::kLength);
	index = Insert(Value("of"));
	assert(index == ConstIndexEmbedded::kOf);
	index = Insert(Value("push"));
	assert(index == ConstIndexEmbedded::kPush);
	index = Insert(Value("pop"));
	assert(index == ConstIndexEmbedded::kPop);
	index = Insert(Value("forEach"));
	assert(index == ConstIndexEmbedded::kForEach);
	index = Insert(Value("map"));
	assert(index == ConstIndexEmbedded::kMap);
	index = Insert(Value("filter"));
	assert(index == ConstIndexEmbedded::kFilter);
	index = Insert(Value("reduce"));
	assert(index == ConstIndexEmbedded::kReduce);

	assert(size() == ConstIndexEmbedded::kEnd);
}

ConstIndex GlobalConstPool::Insert(const Value& value) {
	auto value_ = value;
	return Insert(std::move(value_));
}

ConstIndex GlobalConstPool::Insert(Value&& value) {
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
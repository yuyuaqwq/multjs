#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

#include <mjs/noncopyable.h>
#include <mjs/const_def.h>
#include <mjs/value.h>

namespace mjs {

// 使用分段静态数组，避免resize使得其他运行Context的线程访问const
// 每块静态数组有1024个元素，满了就new新1级数组
class GlobalConstPool : public noncopyable {
private:
	static constexpr size_t kStaticArraySize = 1024;
	using StaticArray = std::array<Value, kStaticArraySize>;

public:
	GlobalConstPool();

	ConstIndex insert(const Value& value);
	ConstIndex insert(Value&& value);

	const Value& get(ConstIndex index) const;
	Value& get(ConstIndex index);

	std::optional<ConstIndex> find(const Value& value);

private:
	std::mutex mutex_;
	std::map<Value, ConstIndex> const_map_;
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	uint32_t const_index_ = 1;
};

class LocalConstPool : public noncopyable {
public:
	LocalConstPool();

	ConstIndex insert(const Value& value);
	ConstIndex insert(Value&& value);

	const Value& get(ConstIndex index) const;
	Value& get(ConstIndex index);

private:
	std::map<Value, ConstIndex> const_map_;
	std::vector<Value*> pool_;
};

} // namespace mjs
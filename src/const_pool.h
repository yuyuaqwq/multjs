#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <mutex>

#include "value.h"

namespace mjs {

// pool使用分段静态数组，避免resize使得其他运行Context的线程访问const
// 如每块静态数组有1024个元素，2级数组满了之后就new一块新静态数组，放回1级数组中
// 索引访问则是idx / 1024找到1级数组索引，idx % 1024找到2级数组索引
class ConstPool {
private:
	static constexpr size_t kStaticArraySize = 1024;
	using StaticArray = std::array<Value, kStaticArraySize>;

public:
	uint32_t New(const Value& value);
	uint32_t New(Value&& value);

	const Value& Get(uint32_t index) const;
	Value& Get(uint32_t index);

private:
	std::mutex mutex_;
	std::map<Value, uint32_t> const_map_;
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	uint32_t const_index_ = 0;
};

} // namespace mjs
#pragma once

#include <vector>
#include <string>
#include <memory>

#include "value.h"

namespace mjs {

// 需要注意的是，pool应该修改为使用分段静态数组，避免resize使得其他运行Context的线程访问const
// 如每块静态数组有1024个元素，满了之后就new一块新静态数组，放回动态数组中
// 索引访问则是idx / 1024找到动态数组索引，idx % 1024找到静态数组索引
class ConstPool {
public:
	uint32_t New(const Value& value);
	uint32_t New(Value&& value);

	// 负数表示从尾部索引起
	const Value& Get(int32_t index) const;
	size_t Size() const noexcept;

private:
	// 常量
	std::map<Value, uint32_t> const_map_;

	std::vector<Value> pool_;
};

} // namespace mjs
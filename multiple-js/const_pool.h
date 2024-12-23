#pragma once

#include <vector>
#include <string>
#include <memory>

#include "value.h"

namespace mjs {

class ConstPool {
public:
	void Push(const Value& value);

	// 负数表示从尾部索引起
	Value& Get(int32_t index);
	size_t Size() const noexcept;

private:
	std::vector<Value> pool_;
};

} // namespace mjs
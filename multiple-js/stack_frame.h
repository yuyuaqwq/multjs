#pragma once

#include <vector>
#include <string>
#include <memory>

#include "value.h"

namespace mjs {

class StackFrame {
public:
	void Push(const Value& value);
	
	Value Pop();

	// 负数表示从尾部索引起
	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);

	size_t Size()  const noexcept;
	void ReSize(size_t s);

private:
	std::vector<Value> stack_;
};

} // namespace mjs
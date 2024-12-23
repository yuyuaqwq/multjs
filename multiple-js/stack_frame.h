#pragma once

#include <vector>
#include <string>
#include <memory>

namespace mjs {

class Value;
class StackFrame {
public:
	void PushI64(int64_t i64);
	
	uint64_t Pop2();

	// 负数表示从尾部索引起
	//std::unique_ptr<Value>& Get(int32_t index);
	//void Set(int32_t index, std::unique_ptr<Value> value);

	//size_t Size()  const noexcept;
	//void ReSize(size_t size);
	//void Clear() noexcept;


private:
	std::vector<uint64_t> stack_;
};

} // namespace mjs
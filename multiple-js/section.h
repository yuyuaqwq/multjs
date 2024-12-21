#pragma once

#include <vector>
#include <string>
#include <memory>

namespace mjs {

class Value;
class ValueSection {
public:
	void Push(std::unique_ptr<Value>&& value);
	std::unique_ptr<Value> Pop();

	// 负数表示从尾部索引起
	std::unique_ptr<Value>& Get(int32_t index);
	void Set(int32_t index, std::unique_ptr<Value> value);

	size_t Size()  const noexcept;
	void ReSize(size_t size);
	void Clear() noexcept;

	void operator=(ValueSection&& vs) noexcept;

private:
	std::vector<std::unique_ptr<Value>> container_;
};

} // namespace mjs
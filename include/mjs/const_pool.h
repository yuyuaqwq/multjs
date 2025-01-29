#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

#include <mjs/const_def.h>
#include <mjs/value.h>

namespace mjs {

// 使用分段静态数组，避免resize使得其他运行Context的线程访问const
// 每块静态数组有1024个元素，满了就new新1级数组
class GlobalConstPool {
private:
	static constexpr size_t kStaticArraySize = 1024;
	using StaticArray = std::array<Value, kStaticArraySize>;

public:
	GlobalConstPool();

	ConstIndex New(const Value& value);
	ConstIndex New(Value&& value);

	const Value& Get(ConstIndex index) const;
	Value& Get(ConstIndex index);

	std::optional<ConstIndex> Find(const Value& value);

private:
	std::mutex mutex_;
	std::map<Value, ConstIndex> const_map_;
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	uint32_t const_index_ = 1;
};

class LocalConstPool {
public:
	LocalConstPool();

	ConstIndex New(const Value& value);
	ConstIndex New(Value&& value);

	const Value& Get(ConstIndex index) const;
	Value& Get(ConstIndex index);

private:
	std::map<Value, ConstIndex> const_map_;
	std::vector<Value*> pool_;
};

} // namespace mjs
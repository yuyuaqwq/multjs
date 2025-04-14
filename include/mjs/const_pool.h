#pragma once

#include <vector>
#include <map>
#include <memory>
#include <mutex>

#include <mjs/noncopyable.h>
#include <mjs/segmented_array.h>
#include <mjs/const_def.h>
#include <mjs/value.h>

namespace mjs {

class GlobalConstPool : public SegmentedArray<Value, ConstIndex, 1024> {
private:
	using Base = SegmentedArray<Value, ConstIndex, 1024>;

public:
	ConstIndex insert(const Value& value) {
		auto value_ = value;
		return insert(std::move(value_));
	}

	ConstIndex insert(Value&& value) {
		auto lock = std::lock_guard(mutex_);

		auto it = map_.find(value);
		if (it != map_.end()) {
			return it->second;
		}

		auto idx = Base::insert(std::move(value));
		auto& val = operator[](idx);

		val.set_const_index(idx);
		auto res = map_.emplace(val, idx);

		//idx = ConstToGlobalIndex(idx);
		return idx;
	}

	std::optional<ConstIndex> find(const Value& value) {
		auto lock = std::lock_guard(mutex_);
		auto it = map_.find(value);
		if (it != map_.end()) {
			return it->second;
		}
		return std::nullopt;
	}

private:
	std::mutex mutex_;
	std::map<Value, ConstIndex> map_;
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
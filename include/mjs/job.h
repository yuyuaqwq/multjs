#pragma once

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/object/object.h>

namespace mjs {

// Job统一在执行后释放
class Job : public noncopyable {
public:
	Job(Value func, Value this_val)
		: func_(std::move(func))
		, this_val_(std::move(this_val)) {}

	Job(Job&& other) noexcept {
		operator=(std::move(other));
	}

	void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) {
		callback(list, func_);
		callback(list, this_val_);
		for (auto& val : argv_) {
			callback(list, val);
		}
	}

	void operator=(Job&& other) noexcept {
		func_ = std::move(other.func_);
		this_val_ = std::move(other.this_val_);
		argv_ = std::move(other.argv_);
	}

	void AddArg(Value value) {
		argv_.emplace_back(std::move(value));
	}

	const auto& func() const { return func_; }
	auto& func() { return func_; }
	const auto& this_val() const { return this_val_; }
	const auto& argv() const { return argv_; }

private:
	Value func_;
	Value this_val_;
	std::vector<Value> argv_;
};

} // namespace mjs
#pragma once

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

// Jobͳһ��ִ�к��ͷ�
class Job : public noncopyable {
public:
	Job(Value func, Value this_val)
		: func_(std::move(func))
		, this_val_(std::move(this_val)) {}

	Job(Job&& other) noexcept {
		operator=(std::move(other));
	}

	void operator=(Job&& other) noexcept {
		func_ = std::move(other.func_);
		this_val_ = std::move(other.this_val_);
		argv_ = std::move(other.argv_);
	}

	const auto& func() const { return func_; }
	const auto& this_val() const { return this_val_; }
	const auto& argv() const { return argv_; }


private:
	Value func_;
	Value this_val_;
	std::vector<Value> argv_;
};

} // namespace mjs
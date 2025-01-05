#pragma once

#include <cstdint>

namespace mjs {

class Value;
class Object {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_count_ = 1;

		properties_ = nullptr;
	}
	virtual ~Object() = default;

	uint32_t ref_count() const {
		return tag_.ref_count_;
	}

	void ref() {
		++tag_.ref_count_;
	}

	void deref() {
		--tag_.ref_count_;
	}

private:
	union {
		uint64_t full_;
		uint32_t ref_count_;
	} tag_;

	Value* properties_;
};
} // namespace mjs
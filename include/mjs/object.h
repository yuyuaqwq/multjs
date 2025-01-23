#pragma once

#include <unordered_map>

namespace mjs {

// using PropertieMap = std::unordered_map<std::string, Value>;
class Value;
class PropertieMap;
class Object {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_count_ = 0;

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
	PropertieMap* properties_;
};

} // namespace mjs
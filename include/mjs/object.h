#pragma once

#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

using PropertyMap = std::unordered_map<std::string, Value>;
class Object : noncopyable {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_count_ = 0;

		property_map_ = nullptr;
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

	void SetProperty(const std::string& name, Value&& val) {
		if (!property_map_) property_map_ = new PropertyMap();
		(*property_map_)[name] = std::move(val);
	}

	Value* GetProperty(const std::string& name) {
		if (!property_map_) return nullptr;
		auto iter = property_map_->find(name);
		if (iter == property_map_->end()) return nullptr;
		return &iter->second;
	}

	void DelProperty(const std::string& name) {
		if (!property_map_) return;
		property_map_->erase(name);
	}

private:
	union {
		uint64_t full_;
		uint32_t ref_count_;
	} tag_;
	PropertyMap* property_map_;
};

} // namespace mjs
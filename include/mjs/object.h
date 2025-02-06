#pragma once

#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/const_def.h>
#include <mjs/value.h>

namespace mjs {

// 
using PropertyMap = std::map<Value, Value>;// std::unordered_map<Value, Value>;
class Object : noncopyable {
public:
	Object() {
		tag_.full_ = 0;
		tag_.ref_count_ = 0;

		property_map_ = nullptr;
	}
	virtual ~Object() {
		if (property_map_) {
			delete property_map_;
		}
		assert(tag_.ref_count_ == 0);
	}

	uint32_t ref_count() const {
		return tag_.ref_count_;
	}

	void ref() {
		++tag_.ref_count_;
	}

	void deref() {
		--tag_.ref_count_;
	}

	void SetProperty(const Value& key, Value&& val) {
		if (!property_map_) property_map_ = new PropertyMap();
		(*property_map_)[key] = std::move(val);
	}

	Value* GetProperty(const Value& key) {
		if (!property_map_) return nullptr;
		auto iter = property_map_->find(key);
		if (iter == property_map_->end()) return nullptr;
		return &iter->second;
	}

	void DelProperty(const Value& key) {
		if (!property_map_) return;
		property_map_->erase(key);
	}

private:
	union {
		uint64_t full_;
		uint32_t ref_count_;
	} tag_;
	PropertyMap* property_map_;
};

} // namespace mjs
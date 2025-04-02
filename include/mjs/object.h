#pragma once

#include <string_view>
#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/const_def.h>
#include <mjs/class_def.h>
#include <mjs/value.h>

namespace mjs {

// 未来考虑优化
// 数组+哈希表
// .方式访问，编译成类似局部变量的数组idx访问
// []方式访问，走哈希表查询得到idx，再查数组

class Runtime;
class Object : public noncopyable {
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

	void Reference() {
		++tag_.ref_count_;
	}

	void Dereference() {
		--tag_.ref_count_;
	}

	void SetProperty(Runtime* runtime, const Value& key, Value&& val);

	Value* GetProperty(Runtime* runtime, const Value& key);

	void DelProperty(Context* context, const Value& key);


	virtual ClassId class_id() const { return ClassId::kBase; }

	auto ref_count() const { return tag_.ref_count_; }

	const auto& prototype() const { return prototype_; }
	void set_prototype(Value prototype) { prototype_ = std::move(prototype); }

protected:
	union {
		uint64_t full_;
		uint32_t ref_count_;
	} tag_;
	Value prototype_;
	PropertyMap* property_map_;
};

} // namespace mjs
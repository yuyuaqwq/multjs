#pragma once

#include <string_view>
#include <map>
#include <unordered_map>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>
#include <mjs/const_def.h>
#include <mjs/class_def.h>
#include <mjs/value.h>

namespace mjs {

// 未来考虑优化
// 数组+哈希表
// .方式访问，编译成类似局部变量的数组idx访问
// []方式访问，走哈希表查询得到idx，再查数组

class Runtime;
class Context;
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {
public:
	Object(Context* context);

	virtual ~Object();

	virtual void ForEachChild(std::function<void(Object&)> callback) {
		// 遍历property_map_
		if (property_map_) {
			for (auto& pair : *property_map_) {
				if (pair.first.IsObject()) {
					callback(pair.first.object());
				}
				if (pair.second.IsObject()) {
					callback(pair.second.object());
				}
			}
		}
	}

	void SetProperty(Context* context, const Value& key, Value&& val);
	Value* GetProperty(Context* context, const Value& key);
	void DelProperty(Context* context, const Value& key);


	void Reference();
	void Dereference();
	void WeakDereference();

	virtual ClassId class_id() const { return ClassId::kBase; }
	auto ref_count() const { return tag_.ref_count_; }
	const auto& prototype() const { return prototype_; }
	void set_prototype(Value prototype) { prototype_ = std::move(prototype); }

protected:
	union {
		uint64_t full_ = 0;
		uint32_t ref_count_;
	} tag_;
	Value prototype_;
	PropertyMap* property_map_ = nullptr;
};

} // namespace mjs
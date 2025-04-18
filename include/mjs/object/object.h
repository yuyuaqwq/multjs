#pragma once

#include <string_view>
#include <map>
#include <unordered_map>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>
#include <mjs/const_def.h>
#include <mjs/value.h>
#include <mjs/class_def/class_def.h>

namespace mjs {

// 未来考虑优化
// 数组+哈希表
// .方式访问，编译成类似局部变量的数组idx访问
// []方式访问，走哈希表查询得到idx，再查数组
// 但是不太可行，因为这个对象的生命周期是不确定的，不能确定会在哪里开始哪里结束
// 编译idx顺序也就无法确定

class Runtime;
class Context;
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {
public:
	Object(Context* context);

	virtual ~Object();

	virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) {
		callback(list, prototype_);
		if (property_map_) {
			for (auto& pair : *property_map_) {
				callback(list, pair.first);
				callback(list, pair.second);
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
	bool gc_mark() { return  tag_.gc_mark_; }
	void set_gc_mark(bool flag) { tag_.gc_mark_ = flag; }
protected:
	union {
		uint64_t full_ = 0;
		struct {
			uint32_t ref_count_;
			uint32_t gc_mark_ : 1;
		};
	} tag_;
	Value prototype_;
	PropertyMap* property_map_ = nullptr;
};

} // namespace mjs
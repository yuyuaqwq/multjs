#pragma once

#include <string_view>
#include <map>
#include <unordered_map>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>
#include <mjs/const_def.h>
#include <mjs/value.h>
#include <mjs/class_def.h>

namespace mjs {

// 保持和js标准一致，属性只能是string或者symbol
// 但其他类型不会自动转换为string，而是抛出异常

class Runtime;
class Context;
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {
public:
	Object(Runtime* runtime);
	Object(Context* context);
	
	virtual ~Object();

	void Reference();
	void Dereference();
	void WeakDereference();

	// 数据成员中有Value，必须重写，否则会内存泄漏
	virtual void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		if (property_map_) {
			for (auto& pair : *property_map_) {
				callback(context, list, pair.second);
			}
		}
	}

	void SetProperty(Context* context, std::string_view key, Value&& value);
	virtual void SetProperty(Context* context, ConstIndex key, Value&& value);
	virtual bool GetProperty(Context* context, ConstIndex key, Value* value);
	virtual bool HasProperty(Context* context, ConstIndex key);
	virtual void DelProperty(Context* context, ConstIndex key);

	virtual void SetComputedProperty(Context* context, const Value& key, Value&& val);
	virtual bool GetComputedProperty(Context* context, const Value& key, Value* value);
	virtual void DelComputedProperty(Context* context, const Value& key);

	virtual Value ToString(Context* context);

	virtual ClassId class_id() const { return ClassId::kObject; }

	template <typename ObjectT>
	const ObjectT& get() const {
		return static_cast<ObjectT&>(*this);
	}

	template <typename ObjectT>
	ObjectT& get() {
		return static_cast<ObjectT&>(*this);
	}

	auto ref_count() const { return tag_.ref_count_; }

	bool gc_mark() { return  tag_.gc_mark_; }
	void set_gc_mark(bool flag) { tag_.gc_mark_ = flag; }

	static Object* New(Context* context) {
		return new Object(context);
	}

protected:
	union {
		uint64_t full_ = 0;
		struct {
			uint32_t ref_count_;
			uint32_t gc_mark_ : 1;
			uint32_t is_const_ : 1;
		};
	} tag_;
	PropertyMap* property_map_ = nullptr;
};

} // namespace mjs
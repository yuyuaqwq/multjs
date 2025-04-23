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

// ���ֺ�js��׼һ�£�����ֻ����string����symbol
// ���������Ͳ����Զ�ת��Ϊstring�������׳��쳣

class Runtime;
class Context;
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {
public:
	Object(Context* context);

	virtual ~Object();

	void Reference();
	void Dereference();
	void WeakDereference();

	virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) {
		callback(list, prototype_);
		if (property_map_) {
			for (auto& pair : *property_map_) {
				callback(list, pair.second);
			}
		}
	}

	virtual Value ToString() {
		return Value("object");
	}

	virtual Object* New(Context* context) {
		auto obj = new Object(context);
		return obj;
	}

	virtual Object* Copy(Object* new_obj, Context* context) {
		new_obj->prototype_ = prototype_;
		if (property_map_) {
			new_obj->property_map_ = new PropertyMap();
			*new_obj->property_map_ = *property_map_;
		}
		return new_obj;
	}

	virtual void SetProperty(Context* context, ConstIndex key, Value&& val);
	virtual Value* GetProperty(Context* context, ConstIndex key);
	virtual void DelProperty(Context* context, ConstIndex key);

	virtual void SetIndexed(Context* context, const Value& key, Value&& val);
	virtual Value* GetIndexed(Context* context, const Value& key);
	virtual void DelIndexed(Context* context, const Value& key);

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
			uint32_t is_const_ : 1;
		};
	} tag_;
	Value prototype_;
	PropertyMap* property_map_ = nullptr;
};

} // namespace mjs
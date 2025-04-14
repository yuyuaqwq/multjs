#pragma once

#include <string_view>
#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>
#include <mjs/const_def.h>
#include <mjs/class_def.h>
#include <mjs/value.h>

namespace mjs {

// δ�������Ż�
// ����+��ϣ��
// .��ʽ���ʣ���������ƾֲ�����������idx����
// []��ʽ���ʣ��߹�ϣ���ѯ�õ�idx���ٲ�����

class Runtime;
class Context;
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {
public:
	Object(Context* context) {
		if (tag_.ref_count_ == 0) {
			// ����context obj����
		}
	}

	virtual ~Object() {
		assert(tag_.ref_count_ == 0);
		if (property_map_) {
			delete property_map_;
		}
	}

	void Reference() {
		++tag_.ref_count_;
	}

	void Dereference() {
		--tag_.ref_count_;
		if (tag_.ref_count_ == 0) {
			delete this;
		}
	}

	void SetProperty(Context* context, const Value& key, Value&& val);

	Value* GetProperty(Context* context, const Value& key);

	void DelProperty(Context* context, const Value& key);


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
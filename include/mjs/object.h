#pragma once

#include <string_view>
#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/const_def.h>
#include <mjs/class_def.h>
#include <mjs/value.h>

namespace mjs {

// δ�������Ż�
// ����+��ϣ��
// .��ʽ���ʣ���������ƾֲ�����������idx����
// []��ʽ���ʣ��߹�ϣ���ѯ�õ�idx���ٲ�����

class Runtime;
class Object : public noncopyable {
public:
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
		uint64_t full_ = 0;
		uint32_t ref_count_;
	} tag_;
	Value prototype_;
	PropertyMap* property_map_ = nullptr;
};

} // namespace mjs
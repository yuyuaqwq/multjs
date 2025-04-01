#pragma once

#include <string>
#include <vector>
#include <mutex>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/property_map.h>
#include <mjs/segmented_array.h>

namespace mjs {

enum class ObjectInternalMethods {
	kGetPrototypeOf = 1 << 0,
	kSetPrototypeOf = 1 << 1,
	kIsExtensible = 1 << 2,
	kPreventExtensions = 1 << 3,
	kGetOwnProperty = 1 << 4,
	kDefineOwnProperty = 1 << 5,
	kHasProperty = 1 << 6,
	kGet = 1 << 7,
	kSet = 1 << 8,
	kDelete = 1 << 9,
	kOwnPropertyKeys = 1 << 10,
};

enum class FunctionInternalMethods {
	kCall = 1 << 1,
};


enum class ClassId {
	kInvalid = 0,
	kBase,
	kNumber,
	kString,
	kArray,
	kGenerator,
	kPromise,

	// δ�����Կ��ǰ�js��ͨ��class�������Ҳ�ŵ��������Ż�
	kCustom,
};

class Object;
class ClassDef : public noncopyable {
public:
	ClassDef(ClassId id, std::string name)
		: id_(id)
		, name_(std::move(name)) {}
	virtual ~ClassDef() = default;

	// �������new���죬��д�ú�����new��ض��󲢷��أ���new ArrayObject()
	virtual Value Constructor(Context* context, uint32_t par_count, StackFrame* stack) { throw std::runtime_error("Types that are not allowed to be constructed."); }

	ClassId id() const { return id_; }

	const auto& property_map() const { return property_map_; }
	auto& property_map() { return property_map_; }

protected:
	ClassId id_;
	std::string name_;
	PropertyMap property_map_;
};

using ClassDefUnique = std::unique_ptr<ClassDef>;

class ClassDefTable : public noncopyable {
public:
	void Register(ClassDefUnique class_def) {
		auto id = class_def->id();
		auto idx = insert(std::move(class_def));
		if (idx != static_cast<uint32_t>(id)) {
			// ���밴ö�ٶ���˳����룬��ȷ����Ч����
			throw std::runtime_error("Class id mismatch.");
		}
	}

	ClassDef& at(ClassId class_id) {
		return *class_defs_.at(static_cast<uint32_t>(class_id));
	}

private:
	ConstIndex insert(ClassDefUnique class_def) {
		auto lock = std::lock_guard(mutex_);
		return class_defs_.insert(std::move(class_def));
	}

private:
	std::mutex mutex_;
	SegmentedArray<ClassDefUnique, uint32_t, 1024> class_defs_;
};

} // namespace mjs
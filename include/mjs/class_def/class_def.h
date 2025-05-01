#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

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
	kNumber,
	kString,
	kSymbol,
	kArray,
	kObject,
	//kNumberObject,
	//kArrayObject,
	kGenerator,
	kPromise,
	kAsync,
	kCustom,
	kModule,
};

class Runtime;
class Object;
class ClassDef : public noncopyable {
public:
	ClassDef(Runtime* runtime, ClassId id, std::string name)
		: id_(id)
		, name_(std::move(name))
		, property_map_(runtime) 
		, static_property_map_(runtime) {}

	virtual ~ClassDef() = default;

	// 如果允许通过原始类型构造，重写该函数，如Symbol()
	virtual Value PrimitiveConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
		throw std::runtime_error(
			"This constructor cannot be called as a function. "
			"Either this is not a callable constructor, "
			"or you need to override PrimitiveConstructor() in the derived class."
		);
	}

	// 如果允许通过new构造，重写该函数，如new ArrayObject()
	virtual Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
		throw std::runtime_error(
			"This constructor cannot be called with 'new'. "
			"Either this is not a constructible function, "
			"or you need to override NewConstructor() in the derived class."
		);
	}

	virtual void SetProperty(Runtime* runtime, ConstIndex key, Value&& val) {
		property_map_.set(runtime, key, std::move(val));
	}

	virtual Value* GetProperty(Runtime* runtime, ConstIndex key) {
		auto iter = property_map_.find(key);
		if (iter != property_map_.end()) {
			return &iter->second;
		}
		return nullptr;
	}

	virtual bool HasProperty(Runtime* runtime, ConstIndex key) {
		auto iter = property_map_.find(key);
		return iter != property_map_.end();
	}

	virtual bool DelProperty(Runtime* runtime, ConstIndex key) {
		auto res = property_map_.erase(runtime, key);
		return res > 0;
	}

	virtual void SetStaticProperty(Runtime* runtime, ConstIndex key, Value&& val) {
		static_property_map_.set(runtime, key, std::move(val));
	}

	virtual Value* GetStaticProperty(Runtime* runtime, ConstIndex key) {
		auto iter = static_property_map_.find(key);
		if (iter != static_property_map_.end()) {
			return &iter->second;
		}
		return nullptr;
	}

	virtual bool HasStaticProperty(Runtime* runtime, ConstIndex key) {
		auto iter = static_property_map_.find(key);
		return iter != static_property_map_.end();
	}

	virtual void DelStaticProperty(Runtime* runtime, ConstIndex key) {
		static_property_map_.erase(runtime, key);
	}


	ClassId id() const { return id_; }
	const auto& name() const { return name_; }

	template<typename ClassDefT>
	ClassDefT& get() {
		return static_cast<ClassDefT&>(*this);
	}

	//const auto& property_map() const { return property_map_; }
	//auto& property_map() { return property_map_; }

protected:
	ClassId id_;
	std::string name_;
	PropertyMap property_map_;
	PropertyMap static_property_map_;
};

using ClassDefUnique = std::unique_ptr<ClassDef>;

class ClassDefTable : public noncopyable {
public:
	void Register(ClassDefUnique class_def) {
		auto id = class_def->id();
		std::string_view name = class_def->name();
		auto idx = insert(std::move(class_def));
		if (idx != static_cast<uint32_t>(id)) {
			// 必须按枚举定义顺序插入，以确保高效查找
			throw std::runtime_error("Class id mismatch.");
		}
		class_def_map_.emplace(name, class_def_arr_[idx].get());
	}

	ClassDef* find(std::string_view class_name) {
		auto iter = class_def_map_.find(class_name);
		if (iter == class_def_map_.end()) {
			return nullptr;
		}
		return iter->second;
	}

	ClassDef& at(ClassId class_id) {
		return *class_def_arr_.at(static_cast<uint32_t>(class_id));
	}

	ClassDef& operator[](ClassId class_id) {
		return *class_def_arr_[(static_cast<uint32_t>(class_id))];
	}

private:
	uint32_t insert(ClassDefUnique class_def) {
		auto lock = std::lock_guard(mutex_);
		return class_def_arr_.insert(std::move(class_def));
	}

private:
	std::mutex mutex_;
	SegmentedArray<ClassDefUnique, uint32_t, 1024> class_def_arr_;
	std::unordered_map<std::string_view, ClassDef*> class_def_map_;
};

} // namespace mjs
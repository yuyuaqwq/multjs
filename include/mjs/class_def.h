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

	// 未来可以考虑把js里通过class定义的类也放到这里做优化
	kCustom,
};

class Object;
class ClassDef : public noncopyable {
public:
	ClassDef(ClassId id, std::string name)
		: id_(id)
		, name_(std::move(name)) {}
	virtual ~ClassDef() = default;

	// 如果允许被new构造，重写该函数，new相关对象并返回，如new ArrayObject()
	virtual Value Constructor(Context* context, uint32_t par_count, StackFrame* stack) { throw std::runtime_error("Types that are not allowed to be constructed."); }

	ClassId id() const { return id_; }

	const auto& property_map() const { return property_map_; }
	auto& property_map() { return property_map_; }

protected:
	ClassId id_;
	std::string name_;
	PropertyMap property_map_;
};

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef()
		: ClassDef(ClassId::kGenerator, "Generator")
	{
		// key到时候优化到runtime的const pool里
		property_map_.NewMethod(Value("next"), Value(ValueType::kGeneratorNext));
	}
};

//class PromiseClassDef : public ClassDef {
//	PromiseClassDef()
//		: ClassDef(ClassId::kPromise, "Promise")
//	{
//		property_map_.NewMethod(Value("resolve"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
//			auto& promise = this_val.promise();
//			promise.Resolve(context);
//			return Value();
//		}));
//		property_map_.NewMethod(Value("reject"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
//			auto& promise = this_val.promise();
//			promise.Reject(context);
//			return Value();
//		}));
//		property_map_.NewMethod(Value("then"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
//			auto& promise = this_val.promise();
//			Value on_fulfilled;
//			Value on_rejected;
//			if (par_count > 0) {
//				on_fulfilled = stack->pop();
//			}
//			if (par_count > 1) {
//				on_rejected = stack->pop();
//			}
//			return promise.Then(context, on_fulfilled, on_rejected);
//		}));
//	}
//
//	virtual Value Constructor(Context* context, uint32_t par_count, StackFrame* stack) override {
//		return Value(new PromiseObject());
//	}
//};

using ClassDefUnique = std::unique_ptr<ClassDef>;

class ClassDefTable : public noncopyable {
public:
	void Register(ClassDefUnique class_def) {
		auto id = class_def->id();
		auto idx = insert(std::move(class_def));
		if (idx != static_cast<uint32_t>(id)) {
			// 必须按枚举定义顺序插入，以确保高效查找
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
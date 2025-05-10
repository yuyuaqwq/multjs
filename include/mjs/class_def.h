#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/property_map.h>

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


enum class ClassId : uint16_t {
	kInvalid = 0,
	kNumber,
	kString,
	kSymbol,
	kObject,
	kArrayObject,
	//kNumberObject,
	//kArrayObject,
	kFunctionObject,
	kGeneratorObject,
	kPromiseObject,
	kAsyncObject,
	kModuleObject,
	kConstructorObject,

	kCustom,
};

class Runtime;
class Object;
class ClassDef : public noncopyable {
public:
	ClassDef(Runtime* runtime, ClassId id, std::string name);

	virtual ~ClassDef() = default;

	void InitConstructor(Runtime* runtime);

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

	virtual void SetProperty(Context* context, Object* obj, ConstIndex key, Value&& val);
	virtual bool GetProperty(Context* context, Object* obj, ConstIndex key, Value* value);
	virtual bool HasProperty(Context* context, Object* obj, ConstIndex key);
	virtual bool DelProperty(Context* context, Object* obj, ConstIndex key);

	ClassId id() const { return id_; }
	const auto& name() const { return name_; }
	const auto& name_string() const { return name_string_; }
	const Value& prototype() const { return prototype_; }

	template<typename ClassDefT>
	ClassDefT& get() {
		return static_cast<ClassDefT&>(*this);
	}

protected:
	ClassId id_;
	ConstIndex name_;
	std::string name_string_;

	Value constructor_object_;
	Value prototype_;
};

using ClassDefUnique = std::unique_ptr<ClassDef>;

} // namespace mjs
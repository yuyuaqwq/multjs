#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>

#include <mjs/const_def.h>
#include <mjs/string.h>

namespace mjs {

enum class ValueType : uint32_t {
	// 字面量
	kUndefined = 0,
	kNull,
	kBoolean,
	kNumber,
	kString,

	// 对象
	kObject,
	kNumberObject,
	kStringObject,
	kArrayObject,
	kFunctionObject,
	kGeneratorObject,
	kPromiseObject,

	// 内部使用
	kI64,
	kU64,

	kStringView, // String优化

	kClassDef,
	kUpValue,
	kFunctionDef,
	kCppFunction,
	kGeneratorNext,
	// kPromiseThen,
};

class Context;
class StackFrame;

class Value;

class Object;
class FunctionObject;
class GeneratorObject;
class PromiseObject;

class ClassDef;
struct UpValue {
	Value* value;
};
class FunctionDef;

class Value {
public:
	using CppFunction = Value(*)(Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack);

public:
	Value();
	explicit Value(std::nullptr_t);
	explicit Value(bool boolean);
	explicit Value(double number);
	explicit Value(const char* string_u8);
	explicit Value(std::string str);

	explicit Value(Object* object);
	explicit Value(FunctionObject* func);
	explicit Value(GeneratorObject* generator);
	explicit Value(PromiseObject* promise);

	explicit Value(int64_t i64);
	explicit Value(int32_t i32);
	explicit Value(uint64_t u64);
	explicit Value(uint32_t u32);

	explicit Value(const UpValue& up_value);

	explicit Value(ClassDef* class_def);
	explicit Value(FunctionDef* function_def);
	explicit Value(CppFunction bridge);

	Value(ValueType type);

	~Value();

	Value(const Value& r);
	Value(Value&& r) noexcept;

	void operator=(const Value& r);
	void operator=(Value&& r) noexcept;

	bool operator<(const Value& rhs) const;
	bool operator>(const Value& rhs) const;
	bool operator==(const Value& rhs) const;
	Value operator+(const Value& rhs) const;
	Value operator-(const Value& rhs) const;
	Value operator*(const Value& rhs) const;
	Value operator/(const Value& rhs) const;
	Value operator-() const;
	Value& operator++();
	Value& operator--();
	Value operator++(int);
	Value operator--(int);

	ValueType type() const;

	double number() const;
	void set_number(double number);

	bool boolean() const;
	void set_boolean(bool boolean);

	const char* string() const;

	Object& object() const;
	template<typename ObjectT>
	ObjectT& object() const {
		return static_cast<ObjectT&>(object());
	}

	FunctionObject& function() const;
	GeneratorObject& generator() const;
	PromiseObject& promise() const;

	int64_t i64() const;
	uint64_t u64() const;

	ClassDef& class_def() const;
	const UpValue& up_value() const;
	FunctionDef& function_def() const;
	CppFunction cpp_function() const;

	ConstIndex const_index() const { return tag_.const_index_; }
	void set_const_index(ConstIndex const_index) { tag_.const_index_ = const_index; }

	bool IsUndefined() const;
	bool IsNull() const;
	bool IsBoolean() const;
	bool IsNumber() const;
	bool IsString() const;

	bool IsObject() const;
	bool IsFunctionObject() const;
	bool IsGeneratorObject() const;
	bool IsPromiseObject() const;

	bool IsI64() const;
	bool IsU64() const;
	bool IsClassDef() const;
	bool IsUpValue() const;
	bool IsFunctionDef() const;
	bool IsCppFunction() const;
	bool IsGeneratorNext() const;

	Value ToString() const;
	Value ToBoolean() const;

private:
	union {
		uint64_t full_ = 0;
		struct {
			ValueType type_ : 7;
			uint32_t read_only_ : 1;	// 用于常量池中的Value，在复制时不会触发引用计数的增加，析构时不会减少引用计数

			// 非0则是来自常量池的value
			ConstIndex const_index_;
		};
	} tag_;
	union {
		uint64_t full_ = 0;

		bool boolean_;
		double f64_;
		String* string_;

		Object* object_;

		int64_t i64_;
		uint64_t u64_;
		const char* string_view_;

		ClassDef* class_def_;
		UpValue up_value_;
		FunctionDef* function_def_;
		CppFunction cpp_func_;
	} value_;
};

using CppFunction = Value::CppFunction;

} // namespace mjs
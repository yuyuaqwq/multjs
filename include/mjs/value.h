#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

#include <mjs/const_def.h>
#include <mjs/string.h>
#include <mjs/symbol.h>
#include <mjs/up_value.h>
#include <mjs/exception.h>

namespace mjs {

enum class ValueType : uint32_t {
	// 字面量
	kUndefined = 0,
	kNull,
	kBoolean,
	kInt64,
	kFloat64,
	kString,
	kSymbol,

	// 对象
	kObject,
	kFloatObject,
	kStringObject,
	kArrayObject,
	kFunctionObject,
	kGeneratorObject,
	kPromiseObject,
	kAsyncObject,
	kModuleObject,

	// 内部使用
	kUInt64,
	// String优化，考虑移除，统一使用kString，移除后string()可以直接返回String*，并且hash能缓存
	kStringView,

	kClassDef,
	kNewConstructor,
	kPrimitiveConstructor,

	kUpValue,

	kFunctionDef,
	kCppFunction,

	kGeneratorNext,

	kPromiseResolve,
	kPromiseReject,

	kReferenceCounter,
};

class Context;
class StackFrame;

class Object;
class FunctionObject;
class GeneratorObject;
class PromiseObject;
class AsyncObject;
class ModuleObject;

class ClassDef;
class FunctionDef;

class Value {
public:
	using CppFunction = Value(*)(Context* context, uint32_t par_count, const StackFrame& stack);

public:
	Value();
	explicit Value(std::nullptr_t);
	explicit Value(bool boolean);
	explicit Value(double number);
	explicit Value(const char* string_u8);
	explicit Value(String* str);
	explicit Value(Symbol* symbol);

	explicit Value(ReferenceCounter* rc);

	explicit Value(Object* object);
	explicit Value(FunctionObject* func);
	explicit Value(GeneratorObject* generator);
	explicit Value(PromiseObject* promise);
	explicit Value(AsyncObject* async);
	explicit Value(ModuleObject* module_);

	explicit Value(int64_t i64);
	explicit Value(int32_t i32);
	explicit Value(uint64_t u64);
	explicit Value(uint32_t u32);

	explicit Value(const UpValue& up_value);

	explicit Value(ClassDef* class_def);
	explicit Value(FunctionDef* function_def);
	explicit Value(CppFunction bridge);

	Value(ValueType type);
	Value(ValueType type, PromiseObject* promise);
	Value(ValueType type, ClassDef* class_def);

	~Value();


	Value(const Value& r);
	Value(Value&& r) noexcept;

	void operator=(const Value& r);
	void operator=(Value&& r) noexcept;

	ptrdiff_t Comparer(const Value& rhs) const;
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

	bool boolean() const;
	void set_boolean(bool boolean);
	const char* string_view() const;
	const String& string() const;
	const Symbol& symbol() const;

	double f64() const;
	void set_float64(double number);
	int64_t i64() const;
	uint64_t u64() const;

	Object& object() const;
	template<typename ObjectT>
	ObjectT& object() const {
		return static_cast<ObjectT&>(object());
	}
	FunctionObject& function() const;
	GeneratorObject& generator() const;
	PromiseObject& promise() const;
	AsyncObject& async() const;
	ModuleObject& module() const;

	ClassDef& class_def() const;
	const UpValue& up_value() const;
	FunctionDef& function_def() const;
	CppFunction cpp_function() const;

	ReferenceCounter& reference_counter() const;
	template<typename ReferenceCounterT>
	ReferenceCounterT& reference_counter() const {
		return *static_cast<ReferenceCounterT*>(reference_counter());
	}

	ConstIndex const_index() const { return tag_.const_index_; }
	void set_const_index(ConstIndex const_index) { tag_.const_index_ = const_index; }

	size_t hash() const;

	bool IsUndefined() const;
	bool IsNull() const;
	bool IsBoolean() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsSymbol() const;

	bool IsReferenceCounter() const;

	// 新对象必须添加到IsObject中，否则会内存泄露
	bool IsObject() const;
	bool IsFunctionObject() const;
	bool IsGeneratorObject() const;
	bool IsPromiseObject() const;
	bool IsAsyncObject() const;
	bool IsModuleObject() const;
	bool IsPromiseResolve() const;
	bool IsPromiseReject() const;

	bool IsFloat() const;
	bool IsInt64() const;
	bool IsUInt64() const;
	bool IsUpValue() const;
	bool IsClassDef() const;
	bool IsFunctionDef() const;
	bool IsCppFunction() const;
	bool IsGeneratorNext() const;
	bool IsIteratorObject() const;

	Value ToString(Context* context) const;
	Value ToBoolean() const;
	Value ToNumber() const;

	bool IsException() const { return tag_.exception_; }
	Value& SetException() { tag_.exception_ = 1; return *this; }

private:
	void Clear();
	void Copy(const Value& r);
	void Move(Value&& r);

private:
	union {
		uint64_t full_ = 0;
		struct {
			ValueType type_ : 15;
			uint32_t exception_ : 1;	// 是否是异常返回
			ConstIndex const_index_;	// 非0则是来自常量池的value
		};
	} tag_;
	union {
		uint64_t full_ = 0;

		bool boolean_;
		double f64_;
		String* string_;
		Symbol* symbol_;

		ReferenceCounter* rc_;

		Object* object_;

		int64_t i64_;
		uint64_t u64_;
		const char* string_view_;

		ClassDef* class_def_;

		UpValue up_value_;

		FunctionDef* function_def_;
		CppFunction cpp_func_;

		ExceptionIdx exception_idx_;
	} value_;
};

using CppFunction = Value::CppFunction;

} // namespace mjs

namespace std {
	template<>
	struct hash<mjs::Value> {
		size_t operator()(const mjs::Value& val) const {
			return val.hash();
		}
	};
}
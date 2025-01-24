#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>

namespace mjs {

enum class ValueType : uint64_t {
	kUndefined = 0,
	kNull,
	kBoolean,
	kNumber,
	kString,
	kObject,

	// 内部使用
	kI64,
	kU64,
	kFunctionBody,
	kFunctionRef,
	kFunctionBridge,
	kUpValue,
};

class StackFrame;

class Object;
class FunctionBodyObject;
class FunctionRefObject;
class UpValueObject;

class Value {
public:
	using FunctionBridgeObject = Value(*)(uint32_t par_count, StackFrame* stack);

public:
	Value();
	explicit Value(std::nullptr_t);
	explicit Value(bool boolean);
	explicit Value(double number);
	explicit Value(int64_t i64);
	explicit Value(int32_t i32);
	explicit Value(uint64_t u64);
	explicit Value(uint32_t u32);
	explicit Value(const char* string_u8, size_t size);
	explicit Value(const std::string string_u8);
	explicit Value(Object* object);
	explicit Value(UpValueObject* up_value);
	explicit Value(FunctionBodyObject* body);
	explicit Value(FunctionRefObject* ref);
	explicit Value(FunctionBridgeObject bridge);

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

	int64_t boolean() const;
	void set_boolean(bool boolean);

	const char* string_u8() const;
	void set_string_u8(const char* string_u8, size_t size);

	Object* object() const;
	template<typename ObjectT>
	ObjectT* object() const {
		return static_cast<ObjectT*>(object());
	}

	int64_t i64() const;
	uint64_t u64() const;

	FunctionBodyObject* function_body() const;
	FunctionRefObject* function_ref() const;
	FunctionBridgeObject function_bridge() const;
	UpValueObject* up_value() const;

private:
	union {
		uint64_t full_ = 0;
		struct {
			ValueType type_ : 4;
			uint64_t read_only_ : 1;	// 用于常量池中的Value，在复制时不会触发引用计数的增加，析构时不会减少引用计数
			uint64_t string_length_ : 32;
		};
	} tag_;
	union {
		bool boolean_;
		double f64_;
		Object* object_;
		Value* value_;
		char string_u8_inline_[8];

		int64_t i64_;
		uint64_t u64_;
	} value_;
};

using FunctionBridgeObject = Value::FunctionBridgeObject;

} // namespace mjs
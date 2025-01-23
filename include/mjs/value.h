#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>

#include "instr.h"
#include "object.h"
#include "arr_obj.h"
#include "up_obj.h"
#include "str_obj.h"

// #include "func_obj.h"


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
	kFunctionBridge,
	kUpValue,
};

class StackFrame;
class FunctionBodyObject;
class Value {
public:
	using FunctionBridgeObject = Value(*)(uint32_t par_count, StackFrame* stack);

public:
	Value() {
		tag_.type_ = ValueType::kUndefined;
		std::memset(&value_, 0, sizeof(value_));
	}

	Value(std::nullptr_t) {
		tag_.type_ = ValueType::kNull;
		std::memset(&value_, 0, sizeof(value_));
	}

	Value(bool boolean) {
		tag_.type_ = ValueType::kBoolean;
		value_.boolean_ = boolean;
	}

	Value(double number) {
		tag_.type_ = ValueType::kNumber;
		value_.f64_ = number;
	}

	Value(int64_t i64) {
		tag_.type_ = ValueType::kI64;
		value_.i64_ = i64;
	}

	Value(int32_t i32) {
		tag_.type_ = ValueType::kI64;
		value_.i64_ = i32;
	}

	Value(uint64_t u64) {
		tag_.type_ = ValueType::kU64;
		value_.u64_ = u64;
	}

	Value(uint32_t u32) {
		tag_.type_ = ValueType::kU64;
		value_.u64_ = u32;
	}

	Value(const char* string_u8, size_t size) {
		tag_.type_ = ValueType::kString;
		set_string_u8(string_u8, size);
	}

	Value(const std::string string_u8) {
		tag_.type_ = ValueType::kString;
		set_string_u8(string_u8.data(), string_u8.size());
	}

	explicit Value(Object* object) {
		tag_.type_ = ValueType::kObject;
		value_.object_ = object;
		value_.object_->ref();
	}

	Value(UpValueObject* up_value) {
		tag_.type_ = ValueType::kUpValue;
		value_.object_ = reinterpret_cast<Object*>(up_value);
	}

	Value(FunctionBodyObject* body) {
		tag_.type_ = ValueType::kFunctionBody;
		value_.object_ = reinterpret_cast<Object*>(body);
	}

	Value(FunctionBridgeObject bridge) {
		tag_.type_ = ValueType::kFunctionBridge;
		value_.object_ = reinterpret_cast<Object*>(bridge);
	}


	~Value() {
		if (type() == ValueType::kString && tag_.string_length_ >= sizeof(value_.string_u8_inline_)
			|| type() == ValueType::kObject) {
			object()->deref();
			if (object()->ref_count() == 0) {
				// 释放对象
				delete object();
			}
		}
	}

	Value(const Value& r) {
		operator=(r);
	}

	Value(Value&& r) noexcept {
		operator=(std::move(r));
	}


	void operator=(const Value& r) {
		tag_.full_ = r.tag_.full_;
		if (type() == ValueType::kString && tag_.string_length_ >= sizeof(value_.string_u8_inline_)
			|| type() == ValueType::kObject) {
			value_.object_ = r.value_.object_;
			object()->ref();
		}
		else {
			value_ = r.value_;
		}
	}

	void operator=(Value&& r) noexcept {
		tag_.full_ = r.tag_.full_;
		value_ = r.value_;
		r.tag_.type_ = ValueType::kUndefined;
	}

	bool operator<(const Value& rhs) const {
		if (type() != rhs.type()) {
			return type() < rhs.type();
		}
		// When types are the same, compare based on the type
		switch (type()) {
		case ValueType::kUndefined:
			return false; // Undefined values are considered equal
		case ValueType::kNull:
			return false; // Null values are considered equal
		case ValueType::kBoolean:
			return boolean() < rhs.boolean();
		case ValueType::kNumber:
			return number() < rhs.number();
		case ValueType::kString: {
			return std::strcmp(string_u8(), rhs.string_u8()) < 0;
		}
		case ValueType::kObject:
			return object() < rhs.object(); // Compare pointers
		
		case ValueType::kI64:
			return i64() < rhs.i64();
		case ValueType::kU64:
			return u64() < rhs.u64();
		case ValueType::kFunctionBody:
		case ValueType::kFunctionBridge:
		case ValueType::kUpValue:
			return value_.object_ < rhs.value_.object_;
		default:
			throw std::runtime_error("Incorrect value type.");
		}
	}

	bool operator>(const Value& rhs) const {
		if (type() != rhs.type()) {
			return type() < rhs.type();
		}
		// When types are the same, compare based on the type
		switch (type()) {
		case ValueType::kUndefined:
			return false; // Undefined values are considered equal
		case ValueType::kNull:
			return false; // Null values are considered equal
		case ValueType::kBoolean:
			return boolean() > rhs.boolean();
		case ValueType::kNumber:
			return number() > rhs.number();
		case ValueType::kString: {
			return std::strcmp(string_u8(), rhs.string_u8()) > 0;
		}
		case ValueType::kObject:
			return object() > rhs.object(); // Compare pointers

		case ValueType::kI64:
			return i64() > rhs.i64();
		case ValueType::kU64:
			return u64() > rhs.u64();
		case ValueType::kFunctionBody:
		case ValueType::kFunctionBridge:
		case ValueType::kUpValue:
			return value_.object_ > rhs.value_.object_;
		default:
			throw std::runtime_error("Incorrect value type.");
		}
	}

	bool operator==(const Value& rhs) const {
		if (type() != rhs.type()) {
			return false;
		}
		switch (type()) {
		case ValueType::kUndefined:
			return true;
		case ValueType::kNull:
			return true;
		case ValueType::kBoolean:
			return boolean() == rhs.boolean();
		case ValueType::kNumber:
			return number() == rhs.number();
		case ValueType::kString: {
			return std::strcmp(string_u8(), rhs.string_u8()) == 0;
		}
		case ValueType::kObject:
			return object() == rhs.object();
		case ValueType::kI64:
			return i64() == rhs.i64();
		case ValueType::kFunctionBody:
		case ValueType::kFunctionBridge:
		case ValueType::kUpValue:
			return value_.object_ == rhs.value_.object_;
		default:
			throw std::runtime_error("Incorrect value type.");
		}
	}

	Value operator+(const Value& rhs) const {
		if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
			return Value(number() + rhs.number());
		}
		else {
			throw std::runtime_error("Addition not supported for these Value types.");
		}
	}

	Value operator-(const Value& rhs) const {
		if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
			return Value(number() - rhs.number());
		}
		else {
			throw std::runtime_error("Subtraction not supported for these Value types.");
		}
	}

	Value operator*(const Value& rhs) const {
		if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
			return Value(number() * rhs.number());
		}
		else {
			throw std::runtime_error("Multiplication not supported for these Value types.");
		}
	}

	Value operator/(const Value& rhs) const {
		if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
			if (rhs.number() == 0) {
				throw std::runtime_error("Division by zero.");
			}
			return Value(number() / rhs.number());
		}
		else {
			throw std::runtime_error("Division not supported for these Value types.");
		}
	}

	Value operator-() const {
		if (type() == ValueType::kNumber) {
			return Value(-number());
		}
		else {
			throw std::runtime_error("Neg not supported for these Value types.");
		}
	}

	Value& operator++() {
		if (type() == ValueType::kNumber) {
			++value_.f64_;
		}
		else {
			throw std::runtime_error("Neg not supported for these Value types.");
		}
		return *this;
	}

	Value& operator--() {
		if (type() == ValueType::kNumber) {
			--value_.f64_;
		}
		else {
			throw std::runtime_error("Neg not supported for these Value types.");
		}
		return *this;
	}

	Value operator++(int) {
		if (type() == ValueType::kNumber) {
			Value old = *this;
			++value_.f64_;
			return old;
		}
		else {
			throw std::runtime_error("Neg not supported for these Value types.");
		}
	}

	Value operator--(int) {
		if (type() == ValueType::kNumber) {
			Value old = *this;
			--value_.f64_;
			return old;
		}
		else {
			throw std::runtime_error("Neg not supported for these Value types.");
		}
	}

	ValueType type() const { return tag_.type_; }

	double number() const { assert(type() == ValueType::kNumber); return value_.f64_; }
	void set_number(double number) { assert(type() == ValueType::kNumber); value_.f64_ = number; }
	int64_t boolean() const { assert(type() == ValueType::kBoolean); return value_.boolean_; }
	void set_boolean(bool boolean) { assert(type() == ValueType::kBoolean); value_.boolean_ = boolean; }
	const char* string_u8() const;
	void set_string_u8(const char* string_u8, size_t size);
	Object* object() const { 
		assert(type() == ValueType::kString || type() == ValueType::kObject); 
		return value_.object_;
	}
	template<typename ObjectT>
	ObjectT* object() const {
		return static_cast<ObjectT*>(object());
	}

	int64_t i64() const { assert(type() == ValueType::kI64); return value_.i64_; }
	uint64_t u64() const { assert(type() == ValueType::kU64); return value_.u64_; }

	FunctionBodyObject* function_body() const { assert(type() == ValueType::kFunctionBody); return reinterpret_cast<FunctionBodyObject*>(value_.object_); }
	FunctionBridgeObject function_bridge() const { assert(type() == ValueType::kFunctionBridge); return reinterpret_cast<FunctionBridgeObject>(value_.object_); }
	UpValueObject* up_value() const { assert(type() == ValueType::kUpValue); return reinterpret_cast<UpValueObject*>(value_.object_); }

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
		char string_u8_inline_[8];

		int64_t i64_;
		uint64_t u64_;
	} value_;
};

using FunctionBridgeObject = Value::FunctionBridgeObject;

} // namespace mjs
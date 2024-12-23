#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>

#include "instr.h"

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

class FunctionBodyObject;
class UpValueObject;
class StackFrame;
class Value {
public:
	using FunctionBridge = Value(*)(uint32_t par_count, StackFrame* stack);

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
		set_string_u8_copy(string_u8, size);
	}

	Value(void* object) {
		tag_.type_ = ValueType::kObject;
		value_.object_ = object;
	}

	Value(const std::string string_u8) {
		tag_.type_ = ValueType::kString;
		set_string_u8_copy(string_u8.data(), string_u8.size());
	}

	Value(FunctionBodyObject* body) {
		tag_.type_ = ValueType::kFunctionBody;
		value_.object_ = body;
	}
	Value(FunctionBridge bridge) {
		tag_.type_ = ValueType::kFunctionBridge;
		value_.object_ = bridge;
	}
	Value(UpValueObject* up_value) {
		tag_.type_ = ValueType::kUpValue;
		value_.object_ = up_value;
	}


	~Value() {
		if (type() == ValueType::kString) {
			if (tag_.string_length_ >= sizeof(value_.string_u8_inline_)) {
				delete[] value_.string_u8_;
			}
		}
		if (type() == ValueType::kObject) {

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
		if (type() == ValueType::kString) {
			if (tag_.string_length_ >= sizeof(value_.string_u8_inline_)) {
				set_string_u8_copy(r.value_.string_u8_, r.tag_.string_length_);
			}
			else {
				value_ = r.value_;
			}
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



	ValueType type() const { return tag_.type_; }

	double number() const { assert(type() == ValueType::kNumber); return value_.f64_; }
	void set_number(double number) { assert(type() == ValueType::kNumber); value_.f64_ = number; }
	int64_t boolean() const { assert(type() == ValueType::kBoolean); return value_.boolean_; }
	void set_boolean(bool boolean) { assert(type() == ValueType::kBoolean); value_.boolean_ = boolean; }
	const char* string_u8() const { 
		assert(type() == ValueType::kString);
		if (tag_.string_length_ < sizeof(value_.string_u8_inline_)) {
			return value_.string_u8_inline_;
		}
		return value_.string_u8_; 
	}
	void* object() const { assert(type() == ValueType::kObject); return value_.object_; }
	template<typename ObjectT>
	ObjectT* object() const { return static_cast<ObjectT*>(object()); }

	int64_t i64() const { assert(type() == ValueType::kI64); return value_.i64_; }
	uint64_t u64() const { assert(type() == ValueType::kU64); return value_.u64_; }

	FunctionBodyObject* function_body() const { assert(type() == ValueType::kFunctionBody); return static_cast<FunctionBodyObject*>(value_.object_); }
	FunctionBridge function_bridge() const { assert(type() == ValueType::kFunctionBridge); return static_cast<FunctionBridge>(value_.object_); }
	UpValueObject* up_value() const { assert(type() == ValueType::kUpValue); return static_cast<UpValueObject*>(value_.object_); }

private:
	void set_string_u8_copy(const char* string_u8, size_t size) {
		if (size < sizeof(value_.string_u8_inline_)) {
			std::memcpy(value_.string_u8_inline_, string_u8, size);
			value_.string_u8_inline_[size] = '\0';
		}
		else {
			auto new_str = new char[size + 1];
			std::memcpy(new_str, string_u8, size);
			new_str[size] = '\0';
			value_.string_u8_ = new_str;
		}
		tag_.string_length_ = size;
	}

	void set_string_u8_shared(const char* string_u8, size_t size) {
		if (size < sizeof(value_.string_u8_inline_)) {
			std::memcpy(value_.string_u8_inline_, string_u8, size);
			value_.string_u8_inline_[size] = '\0';
		}
		else {
			value_.string_u8_ = string_u8;
		}
		tag_.string_length_ = size;
	}

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
		void* object_;
		const char* string_u8_;
		char string_u8_inline_[8];

		int64_t i64_;
		uint64_t u64_;
	} value_;
};

using FunctionBridge = Value::FunctionBridge;
} // namespace mjs
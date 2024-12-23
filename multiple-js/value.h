#pragma once

#include <cassert>

#include <string>
#include <memory>

#include "instr.h"

#include "stack_frame.h"

namespace mjs {

enum class ValueType : uint64_t {
	kUndefined = 0,
	kNull,
	kBoolean,
	kNumber,
	kBigInt,
	kString,
	kObject,
};


class Value {
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
		value_.number_ = number;
	}

	Value(uint64_t big_int) {
		tag_.type_ = ValueType::kBigInt;
		value_.big_int_ = big_int;
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
		tag_.full_ = r.tag_.full_;
		std::memcpy(&value_, &r.value_, sizeof(value_));
		if (type() == ValueType::kString) {
			if (tag_.string_length_ >= sizeof(value_.string_u8_inline_)) {
				set_string_u8_copy(r.value_.string_u8_, r.tag_.string_length_);
			}
		}
	}

	Value(Value&& r) noexcept {
		tag_.full_ = r.tag_.full_;
		std::memcpy(&value_, &r.value_, sizeof(value_));
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
		case ValueType::kBigInt:
			return bit_int() < rhs.bit_int();
		case ValueType::kString: {
			return std::strcmp(string_u8(), rhs.string_u8()) < 0;
		}
		case ValueType::kObject:
			return object() < rhs.object(); // Compare pointers
		default:
			return false; // Fallback for unhandled types
		}
	}

	ValueType type() const { return tag_.type_; }

	double number() const { assert(type() == ValueType::kNumber); return value_.number_; }
	int64_t bit_int() const { assert(type() == ValueType::kBigInt); return value_.big_int_; }
	int64_t boolean() const { assert(type() == ValueType::kBoolean); return value_.boolean_; }
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
			uint64_t string_length_ : 32;
		};
	} tag_;
	union {
		bool boolean_;
		int64_t big_int_;
		double number_;
		void* object_;
		const char* string_u8_;
		char string_u8_inline_[8];
	} value_;
};



class ObjectHeader {

};

// 关于函数的设计
// 函数体就是指令流的封装，只会存放在常量里，定义函数会在常量池创建函数
// 并且会在局部变量表中创建并函数原型，指向函数体，类似语法糖的想法
class FunctionBodyObject : public ObjectHeader {
public:
	explicit FunctionBodyObject(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	ByteCode byte_code;
	StackFrame stack_frame;
};


typedef Value(*FunctionBridgeCall)(uint32_t par_count, StackFrame* stack);
class FunctionBridgeObject : public ObjectHeader {
public:
	explicit FunctionBridgeObject(FunctionBridgeCall func_addr) noexcept;

	FunctionBridgeCall func_addr;
};

class FunctionProtoObject : public ObjectHeader {
public:
	explicit FunctionProtoObject(FunctionBodyObject* value) noexcept;
	explicit FunctionProtoObject(FunctionBridgeObject* value) noexcept;

	union {
		FunctionBodyObject* body_val;
		FunctionBridgeObject* bridge_val;
	};
};

class UpObject : public ObjectHeader {
public:
	UpObject(uint32_t t_index, FunctionBodyObject* func_proto) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_proto;
};

} // namespace mjs
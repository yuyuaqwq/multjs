#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>

#include <mjs/const_def.h>

namespace mjs {

enum class ValueType : uint32_t {
	kUndefined = 0,
	kNull,
	kBoolean,
	kNumber,
	kString,
	kObject,

	// 内部使用
	kI64,
	kU64,

	kUpValue,

	kFunctionBridge,
	kFunctionDef,
	kFunction,
};

class Value;
struct UpValue {
	Value* value;
};

class Object;
class FunctionDefObject;
class FunctionObject;

class StackFrame;

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
	explicit Value(const UpValue& up_value);
	explicit Value(FunctionDefObject* def);
	explicit Value(FunctionObject* func);
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

	bool boolean() const;
	void set_boolean(bool boolean);

	const char* string_u8() const;
	void set_string_u8(const char* string_u8, size_t size);

	const UpValue& up_value() const;

	Object& object() const;
	template<typename ObjectT>
	ObjectT& object() const {
		return static_cast<ObjectT&>(object());
	}

	int64_t i64() const;
	uint64_t u64() const;

	FunctionDefObject* function_def() const;
	FunctionObject* function() const;
	FunctionBridgeObject function_bridge() const;

	ConstIndex const_index() const { return tag_.const_index_; }
	void set_const_index(ConstIndex const_index) { tag_.const_index_ = const_index; }

	Value ToString() const;

private:
	union {
		uint64_t full_ = 0;
		struct {
			ValueType type_ : 4;
			uint32_t read_only_ : 1;	// 用于常量池中的Value，在复制时不会触发引用计数的增加，析构时不会减少引用计数

			// 非0则是来自常量池的value
			ConstIndex const_index_;
		};
	} tag_;
	union {
		bool boolean_;
		double f64_;
		Object* object_;
		
		UpValue up_value_;

		int64_t i64_;
		uint64_t u64_;
	} value_;
};

using FunctionBridgeObject = Value::FunctionBridgeObject;

} // namespace mjs
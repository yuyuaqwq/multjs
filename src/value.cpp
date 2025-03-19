#include <format>

#include <mjs/value.h>

#include <mjs/object.h>
#include <mjs/str_obj.h>

namespace mjs {

Value::Value() {
	tag_.type_ = ValueType::kUndefined;
	std::memset(&value_, 0, sizeof(value_));
}

Value::Value(std::nullptr_t) {
	tag_.type_ = ValueType::kNull;
	std::memset(&value_, 0, sizeof(value_));
}

Value::Value(bool boolean) {
	tag_.type_ = ValueType::kBoolean;
	value_.boolean_ = boolean;
}

Value::Value(double number) {
	tag_.type_ = ValueType::kNumber;
	value_.f64_ = number;
}

Value::Value(int64_t i64) {
	tag_.type_ = ValueType::kI64;
	value_.i64_ = i64;
}

Value::Value(int32_t i32) {
	tag_.type_ = ValueType::kI64;
	value_.i64_ = i32;
}

Value::Value(uint64_t u64) {
	tag_.type_ = ValueType::kU64;
	value_.u64_ = u64;
}

Value::Value(uint32_t u32) {
	tag_.type_ = ValueType::kU64;
	value_.u64_ = u32;
}

Value::Value(const char* string_u8) {
	tag_.type_ = ValueType::kString;
	set_string_u8(string_u8, std::strlen(string_u8));
}

Value::Value(const char* string_u8, size_t size) {
	tag_.type_ = ValueType::kString;
	set_string_u8(string_u8, size);
}

Value::Value(const std::string string_u8) {
	tag_.type_ = ValueType::kString;
	set_string_u8(string_u8.data(), string_u8.size());
}

Value::Value(Object* object) {
	tag_.type_ = ValueType::kObject;
	value_.object_ = object;
	value_.object_->ref();
}

Value::Value(const UpValue& up_value) {
	tag_.type_ = ValueType::kUpValue;
	value_.up_value_ = up_value;
}

Value::Value(FunctionDefObject* def) {
	tag_.type_ = ValueType::kFunctionDef;
	value_.object_ = reinterpret_cast<Object*>(def);
}

Value::Value(FunctionObject* ref) {
	tag_.type_ = ValueType::kFunction;
	value_.object_ = reinterpret_cast<Object*>(ref);
}

Value::Value(FunctionBridgeObject bridge) {
	tag_.type_ = ValueType::kFunctionBridge;
	value_.object_ = reinterpret_cast<Object*>(bridge);
}


Value::~Value() {
	if (type() == ValueType::kString) {
		object().deref();
		if (object().ref_count() == 0) {
			std::destroy_at(&object<StringObject>());
			std::free(&object());
		}
	}
	else if (type() == ValueType::kObject) {
		object().deref();
		if (object().ref_count() == 0) {
			//  Õ∑≈∂‘œÛ
			delete &object();
		}
	}
}

Value::Value(const Value& r) {
	operator=(r);
}

Value::Value(Value&& r) noexcept {
	operator=(std::move(r));
}


void Value::operator=(const Value& r) {
	tag_.full_ = r.tag_.full_;
	if (type() == ValueType::kString || type() == ValueType::kObject) {
		value_.object_ = r.value_.object_;
		object().ref();
	}
	else {
		value_ = r.value_;
	}
}

void Value::operator=(Value&& r) noexcept {
	tag_.full_ = r.tag_.full_;
	value_ = r.value_;
	r.tag_.type_ = ValueType::kUndefined;
}

bool Value::operator<(const Value& rhs) const {
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
		return &object() < &rhs.object(); // Compare pointers

	case ValueType::kI64:
		return i64() < rhs.i64();
	case ValueType::kU64:
		return u64() < rhs.u64();
	case ValueType::kFunctionDef:
	case ValueType::kFunctionBridge:
	case ValueType::kUpValue:
		return value_.up_value_.value < rhs.value_.up_value_.value;
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

bool Value::operator>(const Value& rhs) const {
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
		return &object() > &rhs.object(); // Compare pointers

	case ValueType::kI64:
		return i64() > rhs.i64();
	case ValueType::kU64:
		return u64() > rhs.u64();
	case ValueType::kFunctionDef:
	case ValueType::kFunctionBridge:
	case ValueType::kUpValue:
		return value_.up_value_.value > rhs.value_.up_value_.value;
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

bool Value::operator==(const Value& rhs) const {
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
		return &object() == &rhs.object();
	case ValueType::kI64:
		return i64() == rhs.i64();
	case ValueType::kFunctionDef:
	case ValueType::kFunctionBridge:
	case ValueType::kUpValue:
		return value_.object_ == rhs.value_.object_;
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

Value Value::operator+(const Value& rhs) const {
	if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
		return Value(number() + rhs.number());
	}
	else if (type() == ValueType::kString
		|| rhs.type() == ValueType::kString) {
		return Value(ToString() + rhs.ToString());
	}
	else {
		throw std::runtime_error("Addition not supported for these Value types.");
	}
}

Value Value::operator-(const Value& rhs) const {
	if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
		return Value(number() - rhs.number());
	}
	else {
		throw std::runtime_error("Subtraction not supported for these Value types.");
	}
}

Value Value::operator*(const Value& rhs) const {
	if (type() == ValueType::kNumber && rhs.type() == ValueType::kNumber) {
		return Value(number() * rhs.number());
	}
	else {
		throw std::runtime_error("Multiplication not supported for these Value types.");
	}
}

Value Value::operator/(const Value& rhs) const {
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

Value Value::operator-() const {
	if (type() == ValueType::kNumber) {
		return Value(-number());
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
}

Value& Value::operator++() {
	if (type() == ValueType::kNumber) {
		++value_.f64_;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value& Value::operator--() {
	if (type() == ValueType::kNumber) {
		--value_.f64_;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value Value::operator++(int) {
	if (type() == ValueType::kNumber) {
		Value old = *this;
		++value_.f64_;
		return old;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
}

Value Value::operator--(int) {
	if (type() == ValueType::kNumber) {
		Value old = *this;
		--value_.f64_;
		return old;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
}


ValueType Value::type() const { 
	return tag_.type_;
}


double Value::number() const { 
	assert(type() == ValueType::kNumber);
	return value_.f64_;
}

void Value::set_number(double number) { 
	assert(type() == ValueType::kNumber);
	value_.f64_ = number;
}


bool Value::boolean() const { 
	assert(type() == ValueType::kBoolean); 
	return value_.boolean_;
}

void Value::set_boolean(bool boolean) { 
	assert(type() == ValueType::kBoolean);
	value_.boolean_ = boolean; 
}


const char* Value::string_u8() const {
	assert(type() == ValueType::kString);
	return static_cast<StringObject*>(value_.object_)->str();
}

void Value::set_string_u8(const char* string_u8, size_t size) {
	StringObject* str_obj = static_cast<StringObject*>(std::malloc(sizeof(StringObject) + size));
	std::construct_at(str_obj);
	std::memcpy(str_obj->mutable_str(), string_u8, size);
	str_obj->mutable_str()[size] = '\0';
	value_.object_ = str_obj;
	str_obj->ref();
}


Object& Value::object() const {
	assert(type() == ValueType::kString || type() == ValueType::kObject);
	return *value_.object_;
}


int64_t Value::i64() const { 
	assert(type() == ValueType::kI64); 
	return value_.i64_;
}
uint64_t Value::u64() const { 
	assert(type() == ValueType::kU64);
	return value_.u64_;
}


FunctionDefObject* Value::function_def() const {
	assert(type() == ValueType::kFunctionDef); 
	return reinterpret_cast<FunctionDefObject*>(value_.object_);
}

FunctionObject* Value::function() const {
	assert(type() == ValueType::kFunction);
	return reinterpret_cast<FunctionObject*>(value_.object_);
}

FunctionBridgeObject Value::function_bridge() const { 
	assert(type() == ValueType::kFunctionBridge); 
	return reinterpret_cast<FunctionBridgeObject>(value_.object_); 
}

const UpValue& Value::up_value() const { 
	assert(type() == ValueType::kUpValue); 
	return value_.up_value_;
}

Value Value::ToString() const {
	switch (type()) {
	case ValueType::kUndefined:
		return Value("undefined");
	case ValueType::kNull:
		return Value("null");
	case ValueType::kBoolean:
		return Value(boolean() ? "true" : "false");
	case ValueType::kNumber:
		return Value(std::format("{}", number()));
	case ValueType::kString: {
		return *this;
	}
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}


} // namespace mjs
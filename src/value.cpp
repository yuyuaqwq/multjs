#include <format>

#include <mjs/value.h>
#include <mjs/function_def.h>

#include <mjs/object/object.h>
#include <mjs/object/function_object.h>

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

Value::Value(const char* string_u8) {
	tag_.type_ = ValueType::kStringView;
	value_.string_view_ = string_u8;
}

Value::Value(std::string str) {
	tag_.type_ = ValueType::kString;
	value_.string_ = new String(std::move(str));
	value_.string_->Reference();
}

Value::Value(Object* object) {
	tag_.type_ = ValueType::kObject;
	value_.object_ = object;
	value_.object_->Reference();
}

Value::Value(FunctionObject* function) {
	tag_.type_ = ValueType::kFunctionObject;
	value_.object_ = reinterpret_cast<Object*>(function);
	value_.object_->Reference();
}

Value::Value(GeneratorObject* generator) {
	tag_.type_ = ValueType::kGeneratorObject;
	value_.object_ = reinterpret_cast<Object*>(generator);
	value_.object_->Reference();
}

Value::Value(PromiseObject* promise) {
	tag_.type_ = ValueType::kPromiseObject;
	value_.object_ = reinterpret_cast<Object*>(promise);
	value_.object_->Reference();
}

Value::Value(AsyncObject* async) {
	tag_.type_ = ValueType::kAsyncObject;
	value_.object_ = reinterpret_cast<Object*>(async);
	value_.object_->Reference();
}

Value::Value(ModuleObject* module_) {
	tag_.type_ = ValueType::kModuleObject;
	value_.object_ = reinterpret_cast<Object*>(module_);
	value_.object_->Reference();
}



Value::Value(uint64_t u64) {
	tag_.type_ = ValueType::kU64;
	value_.u64_ = u64;
}

Value::Value(uint32_t u32) {
	tag_.type_ = ValueType::kU64;
	value_.u64_ = u32;
}

Value::Value(const UpValue& up_value) {
	tag_.type_ = ValueType::kUpValue;
	value_.up_value_ = up_value;
}

Value::Value(ClassDef* class_def) {
	tag_.type_ = ValueType::kClassDef;
	value_.class_def_ = class_def;
}

Value::Value(FunctionDef* function_def) {
	tag_.type_ = ValueType::kFunctionDef;
	value_.function_def_ = function_def;
}

Value::Value(CppFunction cpp_func) {
	tag_.type_ = ValueType::kCppFunction;
	value_.cpp_func_ = cpp_func;
}

Value::Value(ValueType type) {
	tag_.type_ = type;
	if (type == ValueType::kGeneratorNext) {
		
	}
	else {
		assert(0);
	}
}

Value::Value(ValueType type, PromiseObject* promise) {
	tag_.type_ = type;
	if (type == ValueType::kPromiseResolve || type == ValueType::kPromiseReject) {
		value_.object_ = reinterpret_cast<Object*>(promise);
		value_.object_->Reference();
	}
	else {
		assert(0);
	}
}


Value::~Value() {
	Clear();
}



Value::Value(const Value& r) {
	Copy(r);
}

Value::Value(Value&& r) noexcept {
	Move(std::move(r));
}


void Value::operator=(const Value& r) {
	Clear();
	Copy(r);
}

void Value::operator=(Value&& r) noexcept {
	Clear();
	Move(std::move(r));
}

ptrdiff_t Value::Comparer(const Value& rhs) const {
	if (type() != rhs.type() && !(IsString() && rhs.IsString())) {
		return static_cast<ptrdiff_t>(type()) - static_cast<ptrdiff_t>(rhs.type());
	}

	switch (type()) {
	case ValueType::kUndefined:
		return 0;
	case ValueType::kNull:
		return 0;
	case ValueType::kBoolean:
		return static_cast<ptrdiff_t>(boolean()) - static_cast<ptrdiff_t>(rhs.boolean());
	case ValueType::kNumber:
		return number() - rhs.number();
	case ValueType::kString:
	case ValueType::kStringView:
		if (string() == rhs.string()) return 0;
		return std::strcmp(string(), rhs.string());
	case ValueType::kObject:
		return &object() - &rhs.object();
	case ValueType::kI64:
		return i64() - rhs.i64();
	case ValueType::kU64:
		return u64() - rhs.u64();
	case ValueType::kFunctionDef:
	case ValueType::kCppFunction:
	case ValueType::kUpValue:
	case ValueType::kClassDef:
		return value_.full_ - rhs.value_.full_;
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

bool Value::operator<(const Value& rhs) const {
	return Comparer(rhs) < 0;
}

bool Value::operator>(const Value& rhs) const {
	return Comparer(rhs) > 0;
}

bool Value::operator==(const Value& rhs) const {
	return Comparer(rhs) == 0;
}

Value Value::operator+(const Value& rhs) const {
	if (IsNumber() && rhs.IsNumber()) {
		return Value(number() + rhs.number());
	}
	else if (IsString() && !rhs.IsString()) {
		return Value(std::format("{}{}", string(), rhs.ToString().string()));
	}
	else if (!IsString() && rhs.IsString()) {
		return Value(std::format("{}{}", ToString().string(), rhs.string()));
	}
	else if (IsString() && rhs.IsString()) {
		return Value(std::format("{}{}", string(), string()));
	}
	else {
		throw std::runtime_error("Addition not supported for these Value types.");
	}
}

Value Value::operator-(const Value& rhs) const {
	if (IsNumber() && rhs.IsNumber()) {
		return Value(number() - rhs.number());
	}
	else {
		throw std::runtime_error("Subtraction not supported for these Value types.");
	}
}

Value Value::operator*(const Value& rhs) const {
	if (IsNumber() && rhs.IsNumber()) {
		return Value(number() * rhs.number());
	}
	else {
		throw std::runtime_error("Multiplication not supported for these Value types.");
	}
}

Value Value::operator/(const Value& rhs) const {
	if (IsNumber() && rhs.IsNumber()) {
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
	if (IsNumber()) {
		return Value(-number());
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
}

Value& Value::operator++() {
	if (IsNumber()) {
		++value_.f64_;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value& Value::operator--() {
	if (IsNumber()) {
		--value_.f64_;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value Value::operator++(int) {
	if (IsNumber()) {
		Value old = *this;
		++value_.f64_;
		return old;
	}
	else {
		throw std::runtime_error("Neg not supported for these Value types.");
	}
}

Value Value::operator--(int) {
	if (IsNumber()) {
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
	assert(IsNumber());
	return value_.f64_;
}

void Value::set_number(double number) { 
	assert(IsNumber());
	value_.f64_ = number;
}


bool Value::boolean() const { 
	assert(IsBoolean()); 
	return value_.boolean_;
}

void Value::set_boolean(bool boolean) { 
	assert(IsBoolean());
	value_.boolean_ = boolean; 
}


const char* Value::string() const {
	assert(IsString());
	if (type() == ValueType::kString) {
		return value_.string_->c_str();
	}
	else if (type() == ValueType::kStringView) {
		return value_.string_view_;
	}
	return nullptr;
}


Object& Value::object() const {
	assert(IsObject());
	return *value_.object_;
}

FunctionObject& Value::function() const {
	assert(IsFunctionObject());
	return *reinterpret_cast<FunctionObject*>(value_.object_);
}

GeneratorObject& Value::generator() const {
	assert(IsGeneratorObject());
	return *reinterpret_cast<GeneratorObject*>(value_.object_);
}

PromiseObject& Value::promise() const {
	assert(IsPromiseObject() || IsPromiseResolve() || IsPromiseReject());
	return *reinterpret_cast<PromiseObject*>(value_.object_);
}

AsyncObject& Value::async() const {
	assert(IsAsyncObject());
	return *reinterpret_cast<AsyncObject*>(value_.object_);
}

ModuleObject& Value::module() const {
	assert(IsModuleObject());
	return *reinterpret_cast<ModuleObject*>(value_.object_);
}

int64_t Value::i64() const {
	assert(IsI64());
	return value_.i64_;
}
uint64_t Value::u64() const {
	assert(IsU64());
	return value_.u64_;
}

ClassDef& Value::class_def() const {
	assert(IsClassDef());
	return *value_.class_def_;
}

const UpValue& Value::up_value() const { 
	assert(IsUpValue()); 
	return value_.up_value_;
}

FunctionDef& Value::function_def() const {
	assert(IsFunctionDef());
	return *value_.function_def_;
}

CppFunction Value::cpp_function() const {
	assert(IsCppFunction());
	return value_.cpp_func_;
}


bool Value::IsUndefined() const {
	return type() == ValueType::kUndefined;
}

bool Value::IsNull() const {
	return type() == ValueType::kNull;
}

bool Value::IsBoolean() const {
	return type() == ValueType::kBoolean;
}

bool Value::IsNumber() const {
	return type() == ValueType::kNumber;
}

bool Value::IsString() const {
	return type() == ValueType::kString
		|| type() == ValueType::kStringView;
}

bool Value::IsObject() const {
	return type() == ValueType::kObject
		|| type() == ValueType::kNumberObject
		|| type() == ValueType::kStringObject
		|| type() == ValueType::kArrayObject
		|| type() == ValueType::kFunctionObject
		|| type() == ValueType::kGeneratorObject
		|| type() == ValueType::kPromiseObject
		|| type() == ValueType::kPromiseResolve
		|| type() == ValueType::kPromiseReject
		|| type() == ValueType::kAsyncObject
		|| type() == ValueType::kModuleObject
		;
}

bool Value::IsFunctionObject() const {
	return type() == ValueType::kFunctionObject;
}

bool Value::IsGeneratorObject() const {
	return type() == ValueType::kGeneratorObject;
}

bool Value::IsPromiseObject() const {
	return type() == ValueType::kPromiseObject;
}

bool Value::IsAsyncObject() const {
	return type() == ValueType::kAsyncObject;
}

bool Value::IsModuleObject() const {
	return type() == ValueType::kModuleObject;
}

bool Value::IsPromiseResolve() const {
	return type() == ValueType::kPromiseResolve;
}

bool Value::IsPromiseReject() const {
	return type() == ValueType::kPromiseReject;
}


bool Value::IsI64() const {
	return type() == ValueType::kI64;
}

bool Value::IsU64() const {
	return type() == ValueType::kU64;
}

bool Value::IsClassDef() const {
	return type() == ValueType::kClassDef;
}

bool Value::IsUpValue() const {
	return type() == ValueType::kUpValue;
}

bool Value::IsFunctionDef() const {
	return type() == ValueType::kFunctionDef;
}

bool Value::IsCppFunction() const {
	return type() == ValueType::kCppFunction;
}

bool Value::IsGeneratorNext() const {
	return type() == ValueType::kGeneratorNext;
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
	case ValueType::kString: 
	case ValueType::kStringView: 
		return *this;
	case ValueType::kI64:
		return Value(std::format("{}", i64()));
	case ValueType::kU64:
		return Value(std::format("{}", u64()));
	case ValueType::kFunctionDef:
		return Value(std::format("function_def:{}", function_def().name()));
	case ValueType::kCppFunction:
		return Value("cpp_function");
	case ValueType::kClassDef:
		return Value(std::format("class_def:{}", class_def().name()));
	case ValueType::kUpValue:
		return up_value().Up().ToString();
	default:
		if (IsObject()) {
			return object().ToString();
		}
		return Value("unknown");
		// throw std::runtime_error("Incorrect value type.");
	}
}

Value Value::ToBoolean() const {
	switch (type()) {
	case ValueType::kUndefined:
		return Value(false);
	case ValueType::kNull:
		return Value(false);
	case ValueType::kBoolean:
		return *this;
	case ValueType::kNumber:
		if (std::isnan(number())) {
			return Value(false);
		}
		return Value(number() == 0);
	case ValueType::kString: {
		return Value(value_.string_->empty());
	}
	case ValueType::kStringView: {
		return Value(string() != nullptr && string()[0] != '\0');
	}
	default:
		throw std::runtime_error("Need to add new types.");
	}
}


void Value::Clear() {
	if (IsString()) {
		if (type() == ValueType::kString) {
			value_.string_->Dereference();
		}
		else if (type() == ValueType::kStringView) {

		}
	}
	else if (IsObject()) {
		object().Dereference();
	}
	else if (IsFunctionDef() && const_index() != 0) {
		delete value_.function_def_;
	}
}

void Value::Copy(const Value& r) {
	tag_.type_ = r.tag_.type_;
	if (r.IsObject()) {
		value_.object_ = r.value_.object_;
		object().Reference();
	}
	else if (r.IsString()) {
		if (r.type() == ValueType::kString) {
			value_.string_ = r.value_.string_;
			value_.string_->Reference();
		}
		else {
			value_.string_view_ = r.value_.string_view_;
		}
	}
	else {
		value_.full_ = r.value_.full_;
	}
	tag_.exception_ = r.tag_.exception_;
}

void Value::Move(Value&& r) {
	tag_.full_ = r.tag_.full_;
	value_ = r.value_;
	r.tag_.type_ = ValueType::kUndefined;
}


} // namespace mjs
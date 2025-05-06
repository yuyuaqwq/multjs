#include <mjs/value.h>

#include <format>

#include <mjs/context.h>
#include <mjs/object.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/function_object.h>

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
	tag_.type_ = ValueType::kFloat64;
	value_.f64_ = number;
}

Value::Value(int64_t i64) {
	tag_.type_ = ValueType::kInt64;
	value_.i64_ = i64;
}

Value::Value(int32_t i32) {
	tag_.type_ = ValueType::kInt64;
	value_.i64_ = i32;
}

Value::Value(const char* string_u8) {
	tag_.type_ = ValueType::kStringView;
	value_.string_view_ = string_u8;
}

Value::Value(String* str) {
	tag_.type_ = ValueType::kString;
	value_.string_ = str;
	value_.string_->Reference();
}

Value::Value(Symbol* symbol) {
	tag_.type_ = ValueType::kSymbol;
	value_.symbol_ = symbol;
	value_.symbol_->Reference();
}

Value::Value(ReferenceCounter* rc) {
	tag_.type_ = ValueType::kReferenceCounter;
	value_.rc_ = rc;
	value_.rc_->Reference();
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

Value::Value(CppModuleObject* module_) {
	tag_.type_ = ValueType::kCppModuleObject;
	value_.object_ = reinterpret_cast<Object*>(module_);
	value_.object_->Reference();
}

Value::Value(ModuleObject* module_) {
	tag_.type_ = ValueType::kModuleObject;
	value_.object_ = reinterpret_cast<Object*>(module_);
	value_.object_->Reference();
}


Value::Value(uint64_t u64) {
	tag_.type_ = ValueType::kUInt64;
	value_.u64_ = u64;
}

Value::Value(uint32_t u32) {
	tag_.type_ = ValueType::kUInt64;
	value_.u64_ = u32;
}

Value::Value(ClassDef* class_def) {
	tag_.type_ = ValueType::kClassDef;
	value_.class_def_ = class_def;
}

Value::Value(ModuleDef* module_def) {
	tag_.type_ = ValueType::kModuleDef;
	value_.module_def_ = module_def;
	value_.module_def_->Reference();
}

Value::Value(FunctionDef* function_def) {
	tag_.type_ = ValueType::kFunctionDef;
	value_.function_def_ = function_def;
	value_.function_def_->Reference();
}

Value::Value(CppFunction cpp_func) {
	tag_.type_ = ValueType::kCppFunction;
	value_.cpp_func_ = cpp_func;
}

Value::Value(ExportVar* export_var) {
	tag_.type_ = ValueType::kExportVar;
	value_.export_var_ = export_var;
}

Value::Value(ClosureVar* closure_var) {
	tag_.type_ = ValueType::kClosureVar;
	value_.closure_var_ = closure_var;
	value_.closure_var_->Reference();
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

Value::Value(ValueType type, ClassDef* class_def) {
	tag_.type_ = type;
	if (type == ValueType::kPrimitiveConstructor || type == ValueType::kNewConstructor) {
		value_.class_def_ = class_def;
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
	if (type() != rhs.type()) {
		if (!(IsString() && rhs.IsString())) {
			return static_cast<ptrdiff_t>(type()) - static_cast<ptrdiff_t>(rhs.type());
		}
	}

	switch (type()) {
	case ValueType::kUndefined:
		return 0;
	case ValueType::kNull:
		return 0;
	case ValueType::kBoolean:
		return static_cast<ptrdiff_t>(boolean()) - static_cast<ptrdiff_t>(rhs.boolean());
	case ValueType::kFloat64:
		return f64() - rhs.f64();
	case ValueType::kString:
		if (rhs.type() == ValueType::kString && value_.string_->hash() != rhs.value_.string_->hash()) {
			return value_.string_->hash() - rhs.value_.string_->hash();
		}
		return std::strcmp(string_view(), rhs.string_view());
	case ValueType::kStringView:
		if (string_view() == rhs.string_view()) return 0;
		return std::strcmp(string_view(), rhs.string_view());
	case ValueType::kSymbol:
		return &symbol() == &rhs.symbol();
	case ValueType::kObject:
		return &object() - &rhs.object();
	case ValueType::kInt64:
		return i64() - rhs.i64();
	case ValueType::kUInt64:
		return u64() - rhs.u64();
	case mjs::ValueType::kPrimitiveConstructor:
	case mjs::ValueType::kNewConstructor:
		// 上面判断了type，这里可以直接比较full
		return value_.full_ - rhs.value_.full_;
	case ValueType::kClassDef:
	case ValueType::kFunctionDef:
	case ValueType::kCppFunction:
	case ValueType::kClosureVar:
		return value_.full_ - rhs.value_.full_;
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

size_t Value::hash() const {
	switch (type()) {
	case mjs::ValueType::kUndefined:
		return 0;
	case mjs::ValueType::kNull:
		return 1;
	case mjs::ValueType::kBoolean:
		return std::hash<bool>()(boolean());
	case mjs::ValueType::kFloat64:
		return std::hash<double>()(f64());
	case mjs::ValueType::kString:
		return value_.string_->hash();
	case mjs::ValueType::kStringView:
		return std::hash<std::string_view>()(string_view());
	case mjs::ValueType::kSymbol:
		return std::hash<const void*>()(&symbol());
	case mjs::ValueType::kObject:
		// 使用对象地址计算哈希
		return std::hash<const void*>()(&object());
	case mjs::ValueType::kInt64:
		return std::hash<int64_t>()(i64());
	case mjs::ValueType::kUInt64:
		return std::hash<uint64_t>()(u64());
	case mjs::ValueType::kPrimitiveConstructor:
	case mjs::ValueType::kNewConstructor:
		// 这里hash暂时不考虑class_def一致但是type不一致的情况
		return std::hash<uint64_t>()(value_.full_);
	case mjs::ValueType::kFunctionDef:
	case mjs::ValueType::kClassDef:
	case mjs::ValueType::kCppFunction:
	case mjs::ValueType::kClosureVar:
		// 使用内部值计算哈希
		return std::hash<uint64_t>()(value_.full_);
	default:
		throw std::runtime_error("Unhashable value type.");
	}
	
}

bool Value::operator<(const Value& rhs) const {
	return Comparer(rhs) < 0;
}

bool Value::operator>(const Value& rhs) const {
	return Comparer(rhs) > 0;
}

bool Value::operator==(const Value& rhs) const {
	if (!const_index().is_invalid() && const_index().is_same_pool(rhs.const_index())) {
		return const_index() == rhs.const_index();
	}
	return Comparer(rhs) == 0;
}

Value Value::operator+(const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(f64() + rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(f64() + double(rhs.i64()));
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::format("{}{}", f64(), rhs.string_view()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(double(i64()) + rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(i64() + rhs.i64());
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::format("{}{}", i64(), rhs.string_view()));
		}
		}
		break;
	}
	case ValueType::kStringView:
	case ValueType::kString: {
		switch (rhs.type()) {
		case ValueType::kFloat64:
			return Value(String::format("{}{}", string_view(), rhs.f64()));
		case ValueType::kInt64: {
			return Value(String::format("{}{}", string_view(), rhs.i64()));
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::format("{}{}", string_view(), rhs.string_view()));
		}
		}
	}
	}
	throw std::runtime_error("Addition not supported for these Value types.");
}

Value Value::operator-(const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(f64() - rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(f64() - double(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(double(i64()) - rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(i64() - rhs.i64());
		}
		}
		break;
	}
	}

	throw std::runtime_error("Subtraction not supported for these Value types.");
	
}

Value Value::operator*(const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(f64() * rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(f64() * double(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(double(i64()) * rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(i64() * rhs.i64());
		}
		}
		break;
	}
	}

	throw std::runtime_error("Multiplication not supported for these Value types.");
}

Value Value::operator/(const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(f64() / rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(f64() / double(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(double(i64()) / rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(double(i64()) / double(rhs.i64()));
		}
		}
		break;
	}
	}
	throw std::runtime_error("Division not supported for these Value types.");
}

Value Value::operator-() const {
	switch (type()) {
	case ValueType::kFloat64: {
		return Value(-f64());
	}
	case ValueType::kInt64: {
		return Value(-i64());
	}
	}
	throw std::runtime_error("Neg not supported for these Value types.");
}

Value& Value::operator++() {
	switch (type()) {
	case ValueType::kFloat64: {
		++value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		++value_.i64_;
		break;
	}
	default:
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value& Value::operator--() {
	switch (type()) {
	case ValueType::kFloat64: {
		--value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		--value_.i64_;
		break;
	}
	default:
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return *this;
}

Value Value::operator++(int) {
	Value old = *this;
	switch (type()) {
	case ValueType::kFloat64: {
		++value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		++value_.i64_;
		break;
	}
	default:
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return old;
}

Value Value::operator--(int) {
	Value old = *this;
	switch (type()) {
	case ValueType::kFloat64: {
		--value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		--value_.i64_;
		break;
	}
	default:
		throw std::runtime_error("Neg not supported for these Value types.");
	}
	return old;
}


ValueType Value::type() const { 
	return tag_.type_;
}


bool Value::boolean() const { 
	assert(IsBoolean()); 
	return value_.boolean_;
}

void Value::set_boolean(bool boolean) { 
	assert(IsBoolean());
	value_.boolean_ = boolean; 
}


const char* Value::string_view() const {
	assert(IsString());
	switch (type()) {
	case ValueType::kString:
		return value_.string_->data();
	case ValueType::kStringView:
		return value_.string_view_;
	default:
		throw std::runtime_error("Non string type.");
	}
}

const String& Value::string() const {
	switch (type()) {
	case ValueType::kString:
		return *value_.string_;
	default:
		throw std::runtime_error("Non string type.");
	}
}

const Symbol& Value::symbol() const {
	assert(IsSymbol());
	return *value_.symbol_;
}


double Value::f64() const {
	assert(IsFloat());
	return value_.f64_;
}

void Value::set_float64(double number) {
	assert(IsFloat());
	value_.f64_ = number;
}

int64_t Value::i64() const {
	assert(IsInt64());
	return value_.i64_;
}
uint64_t Value::u64() const {
	assert(IsUInt64());
	return value_.u64_;
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

CppModuleObject& Value::cpp_module() const {
	assert(IsCppModuleObject());
	return *reinterpret_cast<CppModuleObject*>(value_.object_);
}

ModuleObject& Value::module() const {
	assert(IsModuleObject());
	return *reinterpret_cast<ModuleObject*>(value_.object_);
}


ClassDef& Value::class_def() const {
	assert(IsClassDef() || type() == ValueType::kPrimitiveConstructor || type() == ValueType::kNewConstructor);
	return *value_.class_def_;
}

ExportVar& Value::export_var() const {
	assert(IsExportVar());
	return *value_.export_var_;
}

ClosureVar& Value::closure_var() const { 
	assert(IsClosureVar()); 
	return *value_.closure_var_;
}

ModuleDef& Value::module_def() const {
	assert(IsModuleDef());
	return *value_.module_def_;
}

FunctionDef& Value::function_def() const {
	assert(IsFunctionDef());
	return *value_.function_def_;
}

CppFunction Value::cpp_function() const {
	assert(IsCppFunction());
	return value_.cpp_func_;
}

ReferenceCounter& Value::reference_counter() const {
	assert(IsReferenceCounter());
	return *static_cast<ReferenceCounter*>(value_.rc_);
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
	return IsFloat() || IsInt64();
}

bool Value::IsString() const {
	switch (type()) {
	case ValueType::kString:
	case ValueType::kStringView:
		return true;
	default:
		return false;
	}
}

bool Value::IsSymbol() const {
	return tag_.type_ == ValueType::kSymbol;
}

bool Value::IsReferenceCounter() const {
	switch (type()) {
	case ValueType::kReferenceCounter:
	case ValueType::kString:
	case ValueType::kSymbol:
	case ValueType::kModuleDef:
	case ValueType::kFunctionDef:
	case ValueType::kClosureVar:
		return true;
	default:
		return false;
	}
}

bool Value::IsObject() const {
	switch (type()) {
	case ValueType::kObject:
	case ValueType::kFloatObject:
	case ValueType::kStringObject:
	case ValueType::kArrayObject:
	case ValueType::kFunctionObject:
	case ValueType::kGeneratorObject:
	case ValueType::kPromiseObject:
	case ValueType::kPromiseResolve:
	case ValueType::kPromiseReject:
	case ValueType::kAsyncObject:
	case ValueType::kCppModuleObject:
	case ValueType::kModuleObject:
		return true;
	default:
		return false;
	}
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

bool Value::IsCppModuleObject() const {
	return type() == ValueType::kCppModuleObject;
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


bool Value::IsFloat() const {
	return type() == ValueType::kFloat64;
}

bool Value::IsInt64() const {
	return type() == ValueType::kInt64;
}

bool Value::IsUInt64() const {
	return type() == ValueType::kUInt64;
}

bool Value::IsExportVar() const {
	return type() == ValueType::kExportVar;
}

bool Value::IsClosureVar() const {
	return type() == ValueType::kClosureVar;
}

bool Value::IsClassDef() const {
	return type() == ValueType::kClassDef;
}

bool Value::IsModuleDef() const {
	return type() == ValueType::kModuleDef;
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

Value Value::ToString(Context* context) const {
	switch (type()) {
	case ValueType::kUndefined:
		return Value("undefined");
	case ValueType::kNull:
		return Value("null");
	case ValueType::kBoolean:
		return Value(boolean() ? "true" : "false");
	case ValueType::kFloat64:
		return Value(String::format("{}", f64()));
	case ValueType::kString: 
	case ValueType::kStringView: 
		return *this;
	case ValueType::kInt64:
		return Value(String::format("{}", i64()));
	case ValueType::kUInt64:
		return Value(String::format("{}", u64()));
	case ValueType::kFunctionDef:
		return Value(String::format("function_def:{}", function_def().name()));
	case ValueType::kCppFunction:
		return Value("cpp_function");
	case ValueType::kNewConstructor:
		return Value(String::format("new_constructor:{}", class_def().name()));
	case ValueType::kPrimitiveConstructor:
		return Value(String::format("primitive_constructor:{}", class_def().name()));
	case ValueType::kClosureVar:
		return closure_var().value().ToString(context);
	default:
		if (IsObject()) {
			return object().ToString(context);
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
	case ValueType::kFloat64:
		if (std::isnan(f64())) {
			return Value(false);
		}
		return Value(f64() == 0);
	case ValueType::kString: {
		return Value(value_.string_->empty());
	}
	case ValueType::kStringView: {
		return Value(string_view() != nullptr && string_view()[0] != '\0');
	}
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

Value Value::ToNumber() const {
	switch (type()) {
	case ValueType::kFloat64:
		return Value(f64());
	case ValueType::kInt64: {
		return Value(double(i64()));
	}
	default:
		throw std::runtime_error("Incorrect value type.");
	}
}

void Value::Clear() {
	if (IsObject()) {
		object().Dereference();
	}
	else if (IsReferenceCounter()) {
		value_.rc_->Dereference();
	}
	tag_.type_ = ValueType::kUndefined;
}

void Value::Copy(const Value& r) {
	tag_.type_ = r.tag_.type_;
	tag_.exception_ = r.tag_.exception_;
	tag_.const_index_ = r.tag_.const_index_;
	if (r.IsObject()) {
		value_.object_ = r.value_.object_;
		object().Reference();
	}
	else if (r.IsReferenceCounter()) {
		value_.full_ = r.value_.full_;
		value_.rc_->Reference();
	}
	else {
		value_.full_ = r.value_.full_;
	}
}

void Value::Move(Value&& r) {
	tag_.full_ = r.tag_.full_;
	value_ = r.value_;
	r.tag_.full_ = 0;
	r.tag_.type_ = ValueType::kUndefined;
}


} // namespace mjs
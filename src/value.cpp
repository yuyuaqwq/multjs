#include <mjs/value.h>

#include <cmath>
#include <format>

#include <mjs/const_index_embedded.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/error.h>
#include <mjs/class_def_impl/function_object_class_def.h>
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


Value::Value(Object* object) {
	tag_.type_ = ValueType::kObject;
	value_.object_ = object;
	value_.object_->Reference();
}

Value::Value(ArrayObject* array) {
	tag_.type_ = ValueType::kArrayObject;
	value_.object_ = reinterpret_cast<Object*>(array);
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

Value::Value(ValueType type, AsyncObject* async) {
	tag_.type_ = type;
	if (type == ValueType::kAsyncResolveResume || type == ValueType::kAsyncRejectResume) {
		value_.object_ = reinterpret_cast<Object*>(async);
		value_.object_->Reference();
	}
	else {
		assert(0);
	}
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

Value::Value(ConstructorObject* constructor) {
	tag_.type_ = ValueType::kConstructorObject;
	value_.object_ = reinterpret_cast<Object*>(constructor);
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

//Value::Value(ClassDef* class_def) {
//	tag_.type_ = ValueType::kClassDef;
//	value_.class_def_ = class_def;
//}

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

//Value::Value(ValueType type, ClassDef* class_def) {
//	tag_.type_ = type;
//	if (type == ValueType::kPrimitiveConstructor || type == ValueType::kNewConstructor) {
//		value_.class_def_ = class_def;
//	}
//	else {
//		assert(0);
//	}
//}


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

ptrdiff_t Value::Comparer(Context* context, const Value& rhs) const {
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
	case ValueType::kFunctionDef:
	case ValueType::kModuleDef:
	case ValueType::kCppFunction:
	case ValueType::kClosureVar:
		return value_.full_ - rhs.value_.full_;
	default:
		throw TypeError("Incorrect value type");
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
		return std::hash<const void*>()(&object());
	case mjs::ValueType::kInt64:
		return std::hash<int64_t>()(i64());
	case mjs::ValueType::kUInt64:
		return std::hash<uint64_t>()(u64());
	case mjs::ValueType::kModuleDef:
	case mjs::ValueType::kFunctionDef:
	case mjs::ValueType::kCppFunction:
	case mjs::ValueType::kClosureVar:
		return std::hash<uint64_t>()(value_.full_);
	default:
		throw TypeError("Unhashable value type.");
	}
}

Value Value::LessThan(Context* context, const Value& rhs) const {
	return Value(Comparer(context, rhs) < 0);
}

Value Value::LessThanOrEqual(Context* context, const Value& rhs) const {
	return Value(Comparer(context, rhs) <= 0);
}

Value Value::GreaterThan(Context* context, const Value& rhs) const {
	return Value(Comparer(context, rhs) > 0);
}

Value Value::GreaterThanOrEqual(Context* context, const Value& rhs) const {
	return Value(Comparer(context, rhs) >= 0);
}

Value Value::NotEqualTo(Context* context, const Value& rhs) const {
	return Value(Comparer(context, rhs) != 0);
}

Value Value::EqualTo(Context* context, const Value& rhs) const {
	if (const_index() != kConstIndexInvalid && rhs.const_index() != kConstIndexInvalid) {
		return Value(const_index() == rhs.const_index());
	}
	return Value(Comparer(context, rhs) == 0);
}

Value Value::InstanceOf(Context* context, const Value& rhs) const {
	// instanceof: 检查 rhs 的 prototype 属性是否出现在 this 的原型链上
	// 左操作数(this)是要检查的对象，右操作数(rhs)是构造函数

	// 如果右操作数不是函数对象，抛出 TypeError
	if (!rhs.IsFunctionObject()) {
		throw TypeError("Right-hand side of 'instanceof' is not callable");
	}

	// 如果左操作数不是对象，返回 false
	if (!IsObject() && !IsFunctionObject() && !IsArrayObject()) {
		return Value(false);
	}

	// 获取构造函数的 prototype 属性
	auto& runtime = context->runtime();
	auto constructor_func = rhs.value_.object_;
	Value prototype_value;

	auto& function_object_class_def = context->runtime().class_def_table()[ClassId::kFunctionObject].get<FunctionObjectClassDef>();
	constructor_func->GetProperty(context, ConstIndexEmbedded::kPrototype, &prototype_value);

	// 如果构造函数没有 prototype 属性或 prototype 不是对象，返回 false
	if (!prototype_value.IsObject()) {
		return Value(false);
	}

	auto prototype = prototype_value.value_.object_;

	// 遍历左操作数的原型链
	Value current(this->value_.object_);
	while (current.IsObject() || current.IsFunctionObject() || current.IsArrayObject()) {
		auto current_obj = current.value_.object_;

		// 检查当前原型是否与构造函数的 prototype 相等
		if (current_obj == prototype) {
			return Value(true);
		}

		// 获取下一个原型
		const Value& prototype_ref = current_obj->GetPrototype(context);
		if (!prototype_ref.IsObject()) {
			break;
		}
		current = prototype_ref;
	}

	return Value(false);
}

Value Value::In(Context* context, const Value& rhs) const {
	// in 运算符: 检查属性是否存在于对象或其原型链上
	// 左操作数(this)是属性名，右操作数(rhs)是对象

	// 如果右操作数不是对象，抛出 TypeError
	if (!rhs.IsObject() && !rhs.IsFunctionObject() && !rhs.IsArrayObject()) {
		throw TypeError("Right-hand side of 'in' is not an object");
	}

	auto obj = rhs.value_.object_;

	// 将左操作数转换为属性键
	ConstIndex key_idx;
	if (IsString()) {
		key_idx = context->FindConstOrInsertToLocal(Value(string_view()));
	} 
	// else if (IsSymbol()) {
	//	// Symbol 类型的键需要特殊处理
	//	key_idx = context->FindConstOrInsertToLocal(ToString(context));
	// } 
	else {
		// 其他类型转为字符串
		key_idx = context->FindConstOrInsertToLocal(ToString(context));
	}

	// 检查对象及其原型链上是否有该属性
	return Value(obj->HasProperty(context, key_idx));
}

Value Value::Add(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(f64() + rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(f64() + double(rhs.i64()));
		}
		case ValueType::kUInt64: {
			return Value(f64() + rhs.u64());
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::Format("{}{}", f64(), rhs.string_view()));
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
		case ValueType::kUInt64: {
			return Value(i64() + rhs.u64());
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::Format("{}{}", i64(), rhs.string_view()));
		}
		}
		break;
	}
	case ValueType::kUInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(double(u64()) + rhs.f64());
		}
		case ValueType::kInt64: {
			return Value(u64() + rhs.i64());
		}
		case ValueType::kUInt64: {
			return Value(u64() + rhs.u64());
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::Format("{}{}", u64(), rhs.string_view()));
		}
		}
		break;
	}
	case ValueType::kStringView:
	case ValueType::kString: {
		switch (rhs.type()) {
		case ValueType::kFloat64:
			return Value(String::Format("{}{}", string_view(), rhs.f64()));
		case ValueType::kInt64: {
			return Value(String::Format("{}{}", string_view(), rhs.i64()));
		}
		case ValueType::kUInt64: {
			return Value(String::Format("{}{}", string_view(), rhs.u64()));
		}
		case ValueType::kStringView:
		case ValueType::kString: {
			return Value(String::Format("{}{}", string_view(), rhs.string_view()));
		}
		}
	}
	default:
		return TypeError::Throw(context, "Addition not supported for these Value types");
	}
}

Value Value::Subtract(Context* context, const Value& rhs) const {
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
	default:
		return TypeError::Throw(context, "Subtraction not supported for these Value types");
	}
}

Value Value::Multiply(Context* context, const Value& rhs) const {
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
	default:
		return TypeError::Throw(context, "Subtraction not supported for these Value types");
	}
}

Value Value::Divide(Context* context, const Value& rhs) const {
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
	default:
		return TypeError::Throw(context, "Division not supported for these Value types");
	}
}

Value Value::Modulo(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(fmod(f64(), rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(fmod(f64(), double(rhs.i64())));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(fmod(double(i64()), rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(i64() % rhs.i64());
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Modulo not supported for these Value types");
	}
}

Value Value::LeftShift(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(f64()) << int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(f64()) << int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(i64()) << int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(i64()) << int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Left shift not supported for these Value types");
	}
}

Value Value::RightShift(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(f64()) >> int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(f64()) >> int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(i64()) >> int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(i64()) >> int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Right shift not supported for these Value types");
	}
}


Value Value::UnsignedRightShift(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(uint32_t(f64()) >> int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(uint32_t(f64()) >> int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(uint32_t(i64()) >> int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(uint32_t(i64()) >> int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Unsigned right shift not supported for these Value types");
	}
}
Value Value::BitwiseAnd(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(f64()) & int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(f64()) & int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(i64()) & int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(i64()) & int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Bitwise AND not supported for these Value types");
	}
}

Value Value::BitwiseOr(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(f64()) | int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(f64()) | int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(i64()) | int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(i64()) | int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Bitwise OR not supported for these Value types");
	}
}

Value Value::BitwiseXor(Context* context, const Value& rhs) const {
	switch (type()) {
	case ValueType::kFloat64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(f64()) ^ int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(f64()) ^ int32_t(rhs.i64()));
		}
		}
		break;
	}
	case ValueType::kInt64: {
		switch (rhs.type()) {
		case ValueType::kFloat64: {
			return Value(int32_t(i64()) ^ int32_t(rhs.f64()));
		}
		case ValueType::kInt64: {
			return Value(int32_t(i64()) ^ int32_t(rhs.i64()));
		}
		}
		break;
	}
	default:
		return TypeError::Throw(context, "Bitwise XOR not supported for these Value types");
	}
}

Value Value::BitwiseNot(Context* context) const {
	switch (type()) {
	case ValueType::kFloat64: {
		return Value(~int32_t(f64()));
	}
	case ValueType::kInt64: {
		return Value(~int32_t(i64()));
	}
	default:
		return TypeError::Throw(context, "Bitwise NOT not supported for these Value types");
	}
}



Value Value::Negate(Context* context) const {
	switch (type()) {
	case ValueType::kFloat64: {
		return Value(-f64());
	}
	case ValueType::kInt64: {
		return Value(-i64());
	}
	default:
		return TypeError::Throw(context, "Negation not supported for these Value types");
	}
}

Value Value::Typeof(Context* context) const {
	switch (type()) {
	case ValueType::kUndefined:
		return Value("undefined");
	case ValueType::kNull:
		return Value("object");
	case ValueType::kBoolean:
		return Value("boolean");
	case ValueType::kFloat64:
	case ValueType::kInt64:
	case ValueType::kUInt64:
		return Value("number");
	case ValueType::kString:
	case ValueType::kStringView:
		return Value("string");
	case ValueType::kObject:
	case ValueType::kArrayObject:
	case ValueType::kModuleObject:
	case ValueType::kCppModuleObject:
		return Value("object");
	case ValueType::kFunctionObject:
	case ValueType::kConstructorObject:
	case ValueType::kAsyncObject:
		return Value("function");
	case ValueType::kGeneratorObject:
	case ValueType::kPromiseObject:
		return Value("object");
	case ValueType::kSymbol:
		return Value("symbol");
	default:
		return Value("unknown");
	}
}

Value Value::Increment(Context* context) {
	switch (type()) {
	case ValueType::kFloat64: {
		++value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		++value_.i64_;
		break;
	}
	case ValueType::kUInt64: {
		++value_.u64_;
		break;
	}
	default:
		return TypeError::Throw(context, "Increment not supported for these Value types");
	}
	return *this;
}

Value Value::Decrement(Context* context) {
	switch (type()) {
	case ValueType::kFloat64: {
		--value_.f64_;
		break;
	}
	case ValueType::kInt64: {
		--value_.i64_;
		break;
	}
	case ValueType::kUInt64: {
		--value_.u64_;
		break;
	}
	default:
		return TypeError::Throw(context, "Decrement not supported for these Value types");
	}
	return *this;
}

Value Value::PostIncrement(Context* context) {
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
		return TypeError::Throw(context, "Post increment not supported for these Value types");
	}
	return old;
}

Value Value::PostDecrement(Context* context) {
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
		return TypeError::Throw(context, "Post decrement not supported for these Value types");
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
		throw TypeError("Non string type");
	}
}

const String& Value::string() const {
	switch (type()) {
	case ValueType::kString:
		return *value_.string_;
	default:
		throw TypeError("Non string type");
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

ArrayObject& Value::array() const {
	assert(IsArrayObject());
	return *reinterpret_cast<ArrayObject*>(value_.object_);
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
	assert(IsAsyncObject() || IsAsyncResolveResume() || IsAsyncRejectResume());
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

ConstructorObject& Value::constructor() const {
	assert(IsConstructorObject());
	return *reinterpret_cast<ConstructorObject*>(value_.object_);
}

//ClassDef& Value::class_def() const {
//	assert(IsClassDef() || type() == ValueType::kPrimitiveConstructor || type() == ValueType::kNewConstructor);
//	return *value_.class_def_;
//}

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
	return IsFloat() || IsInt64() || IsUInt64();
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

bool Value::IsStringView() const {
	return type() == ValueType::kStringView;
}

bool Value::IsSymbol() const {
	return tag_.type_ == ValueType::kSymbol;
}

bool Value::IsReferenceCounter() const {
	switch (type()) {
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

void Value::ReferenceCounterInc() {
	switch (type()) {
	case ValueType::kString:
		value_.string_->Reference();
		break;
	case ValueType::kSymbol:
		value_.symbol_->Reference();
		break;
	case ValueType::kModuleDef:
		value_.module_def_->Reference();
		break;
	case ValueType::kFunctionDef:
		value_.function_def_->Reference();
		break;
	case ValueType::kClosureVar:
		value_.closure_var_->Reference();
		break;
	}
}

void Value::ReferenceCounterDec() {
	switch (type()) {
	case ValueType::kString:
		value_.string_->Dereference();
		break;
	case ValueType::kSymbol:
		value_.symbol_->Dereference();
		break;
	case ValueType::kModuleDef:
		value_.module_def_->Dereference();
		break;
	case ValueType::kFunctionDef:
		value_.function_def_->Dereference();
		break;
	case ValueType::kClosureVar:
		value_.closure_var_->Dereference();
		break;
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
	case ValueType::kAsyncResolveResume:
	case ValueType::kAsyncRejectResume:
	case ValueType::kCppModuleObject:
	case ValueType::kModuleObject:
	case ValueType::kConstructorObject:
		return true;
	default:
		return false;
	}
}

bool Value::IsArrayObject() const {
	return type() == ValueType::kArrayObject;
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

bool Value::IsAsyncResolveResume() const {
	return type() == ValueType::kAsyncResolveResume;
}

bool Value::IsAsyncRejectResume() const {
	return type() == ValueType::kAsyncRejectResume;
}

bool Value::IsCppModuleObject() const {
	return type() == ValueType::kCppModuleObject;
}

bool Value::IsModuleObject() const {
	return type() == ValueType::kModuleObject;
}

bool Value::IsConstructorObject() const {
	return type() == ValueType::kConstructorObject;
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

//bool Value::IsClassDef() const {
//	return type() == ValueType::kClassDef;
//}

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
		return Value(String::Format("{}", f64()));
	case ValueType::kString: 
	case ValueType::kStringView: 
		return *this;
	case ValueType::kInt64:
		return Value(String::Format("{}", i64()));
	case ValueType::kUInt64:
		return Value(String::Format("{}", u64()));
	case ValueType::kModuleDef:
		return Value(String::Format("module_def:{}", module_def().name()));
	case ValueType::kModuleObject:
		return Value(String::Format("module_object:{}", module().module_def().name()));
	case ValueType::kFunctionDef:
		return Value(String::Format("function_def:{}", function_def().name()));
	case ValueType::kCppFunction:
		return Value("cpp_function");
	case ValueType::kClosureVar:
		return closure_var().value().ToString(context);
	default:
		if (IsObject()) {
			return object().ToString(context);
		}
		return TypeError::Throw(context, "Incorrect value type");
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
		return Value(f64() != 0);
	case ValueType::kString: {
		return Value(!value_.string_->empty());
	}
	case ValueType::kStringView: {
		return Value(string_view() != nullptr && string_view()[0] != '\0');
	}
	default:
		if (IsObject()) {
			return Value(true);
		}
		throw TypeError("Incorrect value type");
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
		throw TypeError("Incorrect value type");
	}
}

Value Value::ToInt64() const {
	switch (type()) {
	case ValueType::kFloat64:
		return Value(int64_t(f64()));
	case ValueType::kInt64: {
		return *this;
	}
	case ValueType::kUInt64: {
		return Value(int64_t(u64()));
	}
	default:
		throw TypeError("Incorrect value type");
	}
}

Value Value::ToUInt64() const {
	switch (type()) {
	case ValueType::kFloat64:
		return Value(uint64_t(f64()));
	case ValueType::kInt64: {
		return Value(uint64_t(i64()));
	}
	case ValueType::kUInt64: {
		return *this;
	}
	default:
		throw TypeError("Incorrect value type");
	}
}

Value Value::ToObject() const {
	switch (type()) {
	case ValueType::kObject:
		return Value(this);
	default:
		throw TypeError("Incorrect value type");
	}
}



FunctionDefBase& Value::ToFunctionDefBase() const {
	switch (type()) {
	case ValueType::kModuleObject: {
		return module().module_def();
	}
	case ValueType::kModuleDef: {
		return module_def();
	}
	case ValueType::kFunctionObject: {
		return function().function_def();
	}
	case ValueType::kFunctionDef: {
		return function_def();
	}
	default:
		throw TypeError("Incorrect value type");
	}
}
ModuleDef& Value::ToModuleDef() const {
	switch (type()) {
	case ValueType::kModuleObject:
		return module().module_def();
	case ValueType::kModuleDef: {
		return module_def();
	}
	default:
		throw TypeError("Incorrect value type");
	}
}

FunctionDef& Value::ToFunctionDef() const {
	switch (type()) {
	case ValueType::kFunctionObject:
		return function().function_def();
	case ValueType::kFunctionDef: {
		return function_def();
	}
	default:
		throw TypeError("Incorrect value type");
	}
}



void Value::Clear() {
	if (IsObject()) {
		object().Dereference();
	}
	else if (IsReferenceCounter()) {
		ReferenceCounterDec();
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
		ReferenceCounterInc();
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
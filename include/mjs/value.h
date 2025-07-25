#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

#include <mjs/constant.h>
#include <mjs/string.h>
#include <mjs/symbol.h>
#include <mjs/exception.h>

namespace mjs {

// 有效范围：0 ~ 65535
// mjs保留：0 ~ 1024 
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
	kCppModuleObject,
	kModuleObject,
	kConstructorObject,

	// 内部使用
	kUInt64,
	// String优化，考虑移除，统一使用kString，移除后string()可以直接返回String*，并且hash能缓存
	kStringView,

	// kClassDef,
	// kNewConstructor,
	// kPrimitiveConstructor,

	kModuleDef,
	kFunctionDef,
	kCppFunction,
	kExportVar,
	kClosureVar,

	kGeneratorNext,

	kAsyncResolveResume,
	kAsyncRejectResume,

	kPromiseResolve,
	kPromiseReject,
};

class Context;
class StackFrame;

class Object;
class ArrayObject;
class FunctionObject;
class GeneratorObject;
class PromiseObject;
class AsyncObject;
class CppModuleObject;
class ModuleObject;
class ConstructorObject;

class ClassDef;
class FunctionDefBase;
class ModuleDef;
class FunctionDef;
class ExportVar;
class ClosureVar;

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

	explicit Value(Object* object);
	explicit Value(ArrayObject* array);
	explicit Value(FunctionObject* function);
	explicit Value(GeneratorObject* generator);
	explicit Value(PromiseObject* promise);
	explicit Value(AsyncObject* async);
	explicit Value(ValueType type, AsyncObject* async);
	explicit Value(CppModuleObject* module_);
	explicit Value(ModuleObject* module_);
	explicit Value(ConstructorObject* module_);

	explicit Value(int64_t i64);
	explicit Value(int32_t i32);
	explicit Value(uint64_t u64);
	explicit Value(uint32_t u32);

	// explicit Value(const UpValue& up_value);

	// explicit Value(ClassDef* class_def);
	explicit Value(ModuleDef* module_def);
	explicit Value(FunctionDef* function_def);
	explicit Value(CppFunction bridge);
	explicit Value(ExportVar* export_var);
	explicit Value(ClosureVar* closure_var);

	Value(ValueType type);
	Value(ValueType type, PromiseObject* promise);
	// Value(ValueType type, ClassDef* class_def);

	~Value();


	Value(const Value& r);
	Value(Value&& r) noexcept;

	void operator=(const Value& r);
	void operator=(Value&& r) noexcept;

	bool operator==(const Value& r) const {
		return Comparer(nullptr, r) == 0;
	}

	ptrdiff_t Comparer(Context* context, const Value& rhs) const;
	Value LessThan(Context* context, const Value& rhs) const;
	Value LessThanOrEqual(Context* context, const Value& rhs) const;
	Value GreaterThan(Context* context, const Value& rhs) const;
	Value GreaterThanOrEqual(Context* context, const Value& rhs) const;
	Value NotEqualTo(Context* context, const Value& rhs) const;
	Value EqualTo(Context* context, const Value& rhs) const;

	Value Add(Context* context, const Value& rhs) const;
	Value Subtract(Context* context, const Value& rhs) const;
	Value Multiply(Context* context, const Value& rhs) const;
	Value Divide(Context* context, const Value& rhs) const;
	Value LeftShift(Context* context, const Value& rhs) const;
	Value RightShift(Context* context, const Value& rhs) const;
	Value BitwiseAnd(Context* context, const Value& rhs) const;
	Value BitwiseOr(Context* context, const Value& rhs) const;
	Value Negate(Context* context) const;
	Value Increment(Context* context);
	Value Decrement(Context* context);
	Value PostIncrement(Context* context);
	Value PostDecrement(Context* context);

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
	ArrayObject& array() const;
	FunctionObject& function() const;
	GeneratorObject& generator() const;
	PromiseObject& promise() const;
	AsyncObject& async() const;
	CppModuleObject& cpp_module() const;
	ModuleObject& module() const;
	ConstructorObject& constructor() const;

	// ClassDef& class_def() const;
	ModuleDef& module_def() const;
	FunctionDef& function_def() const;
	CppFunction cpp_function() const;
	ExportVar& export_var() const;
	ClosureVar& closure_var() const;
	
	ConstIndex const_index() const { return tag_.const_index_; }
	void set_const_index(ConstIndex const_index) { tag_.const_index_ = const_index; }

	size_t hash() const;

	bool IsUndefined() const;
	bool IsNull() const;
	bool IsBoolean() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsStringView() const;
	bool IsSymbol() const;

	bool IsReferenceCounter() const;
	void ReferenceCounterInc();
	void ReferenceCounterDec();


	// 新对象必须添加到IsObject中，否则会内存泄露
	bool IsObject() const;
	bool IsArrayObject() const;
	bool IsFunctionObject() const;
	bool IsGeneratorObject() const;
	bool IsPromiseObject() const;
	bool IsAsyncObject() const;
	bool IsAsyncResolveResume() const;
	bool IsAsyncRejectResume() const;
	bool IsCppModuleObject() const;
	bool IsModuleObject() const;
	bool IsConstructorObject() const;
	bool IsPromiseResolve() const;
	bool IsPromiseReject() const;
	
	bool IsFloat() const;
	bool IsInt64() const;
	bool IsUInt64() const;
	
	// bool IsClassDef() const;
	bool IsModuleDef() const;
	bool IsFunctionDef() const;
	bool IsCppFunction() const;
	bool IsExportVar() const;
	bool IsClosureVar() const;

	bool IsGeneratorNext() const;
	bool IsIteratorObject() const;

	Value ToString(Context* context) const;
	Value ToBoolean() const;
	Value ToNumber() const;
	Value ToInt64() const;
	Value ToUInt64() const;
	const ModuleDef& ToModuleDef() const;
	const FunctionDef& ToFunctionDef() const;

	bool IsException() const { return tag_.exception_; }
	Value& SetException() { tag_.exception_ = 1; return *this; }

	bool GetProperty(Context* context, ConstIndex key, Value* value);

	static std::string TypeToString(ValueType type) {
		switch (type) {
		case ValueType::kUndefined:
			return "undefined";
		case ValueType::kNull:
			return "null";
		case ValueType::kBoolean:
			return "boolean";
		case ValueType::kFloat64:
			return "float64";
		case ValueType::kInt64:
			return "int64";
		case ValueType::kUInt64:
			return "uint64";
		case ValueType::kString:
			return "string";
		case ValueType::kStringView:
			return "string_view";
		case ValueType::kSymbol:
			return "symbol";
		case ValueType::kObject:
			return "objerct";
		case ValueType::kFunctionDef:
			return "function_def";
		case ValueType::kCppFunction:
			return "cpp_function";
		case ValueType::kClosureVar:
			return "closure_var";
		default:
			throw std::runtime_error("Incorrect value type.");
		}
	}

private:
	void Clear();
	void Copy(const Value& r);
	void Move(Value&& r);

private:
	union {
		uint64_t full_ = 0;
		struct {
			ValueType type_ : 16;
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

		Object* object_;


		int64_t i64_;
		uint64_t u64_;
		const char* string_view_;

		ClassDef* class_def_;

		ModuleDef* module_def_;
		FunctionDef* function_def_;
		CppFunction cpp_func_;
		ExportVar* export_var_;
		ClosureVar* closure_var_;

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
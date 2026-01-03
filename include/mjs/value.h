/**
 * @file value.h
 * @brief JavaScript值类型系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了JavaScript引擎中的值类型系统，包括所有基本类型和对象类型的
 * 枚举定义和值表示机制。
 */

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

/**
 * @enum ValueType
 * @brief JavaScript 值类型枚举
 *
 * 定义了 JavaScript 引擎支持的所有值类型，包括基本类型和对象类型。
 * 有效范围：0 ~ 65535，mjs 保留：0 ~ 1024
 */
enum class ValueType : uint32_t {
	/** @brief 未定义类型 */
	kUndefined = 0,
	/** @brief 空类型 */
	kNull,
	/** @brief 布尔类型 */
	kBoolean,
	/** @brief 64位整数类型 */
	kInt64,
	/** @brief 64位浮点数类型 */
	kFloat64,
	/** @brief 字符串类型 */
	kString,
	/** @brief 符号类型 */
	kSymbol,

	/** @brief 普通对象类型 */
	kObject,
	/** @brief 浮点数对象类型 */
	kFloatObject,
	/** @brief 字符串对象类型 */
	kStringObject,
	/** @brief 数组对象类型 */
	kArrayObject,
	/** @brief 函数对象类型 */
	kFunctionObject,
	/** @brief 生成器对象类型 */
	kGeneratorObject,
	/** @brief Promise 对象类型 */
	kPromiseObject,
	/** @brief 异步对象类型 */
	kAsyncObject,
	/** @brief C++ 模块对象类型 */
	kCppModuleObject,
	/** @brief 模块对象类型 */
	kModuleObject,
	/** @brief 构造函数对象类型 */
	kConstructorObject,

	/** @brief 64位无符号整数类型（内部使用） */
	kUInt64,
	/** @brief 字符串视图类型（内部使用，考虑移除） */
	kStringView,

	/** @brief 模块定义类型（内部使用） */
	kModuleDef,
	/** @brief 函数定义类型（内部使用） */
	kFunctionDef,
	/** @brief C++ 函数类型（内部使用） */
	kCppFunction,
	/** @brief 导出变量类型（内部使用） */
	kExportVar,
	/** @brief 闭包变量类型（内部使用） */
	kClosureVar,

	/** @brief 生成器下一个值类型（内部使用） */
	kGeneratorNext,

	/** @brief 异步解析恢复类型（内部使用） */
	kAsyncResolveResume,
	/** @brief 异步拒绝恢复类型（内部使用） */
	kAsyncRejectResume,

	/** @brief Promise 解析类型（内部使用） */
	kPromiseResolve,
	/** @brief Promise 拒绝类型（内部使用） */
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

/**
 * @class Value
 * @brief JavaScript 值类型包装类
 *
 * 提供 JavaScript 所有值类型的统一表示和操作接口，包括：
 * - 基本类型（undefined、null、boolean、number、string、symbol）
 * - 对象类型（object、array、function、generator、promise 等）
 * - 内部类型（模块定义、函数定义、闭包变量等）
 *
 * @note 使用联合体存储不同类型的值，通过类型标签区分
 * @warning 类型转换操作需要确保类型匹配，否则会抛出异常
 * @see ValueType 值类型枚举
 */
class Value {
public:
	/** @brief C++ 函数类型定义 */
	using CppFunction = Value(*)(Context* context, uint32_t par_count, const StackFrame& stack);

public:
	/** @brief 默认构造函数，创建 undefined 值 */
	Value();
	/** @brief nullptr 构造函数，创建 null 值 */
	explicit Value(std::nullptr_t);
	/** @brief 布尔值构造函数 */
	explicit Value(bool boolean);
	/** @brief 浮点数构造函数 */
	explicit Value(double number);
	/** @brief C 字符串构造函数 */
	explicit Value(const char* string_u8);
	/** @brief 字符串指针构造函数 */
	explicit Value(String* str);
	/** @brief 符号指针构造函数 */
	explicit Value(Symbol* symbol);

	/** @brief 普通对象构造函数 */
	explicit Value(Object* object);
	/** @brief 数组对象构造函数 */
	explicit Value(ArrayObject* array);
	/** @brief 函数对象构造函数 */
	explicit Value(FunctionObject* function);
	/** @brief 生成器对象构造函数 */
	explicit Value(GeneratorObject* generator);
	/** @brief Promise 对象构造函数 */
	explicit Value(PromiseObject* promise);
	/** @brief 异步对象构造函数 */
	explicit Value(AsyncObject* async);
	/** @brief 指定类型的异步对象构造函数 */
	explicit Value(ValueType type, AsyncObject* async);
	/** @brief C++ 模块对象构造函数 */
	explicit Value(CppModuleObject* module_);
	/** @brief 模块对象构造函数 */
	explicit Value(ModuleObject* module_);
	/** @brief 构造函数对象构造函数 */
	explicit Value(ConstructorObject* module_);

	/** @brief 64位整数构造函数 */
	explicit Value(int64_t i64);
	/** @brief 32位整数构造函数 */
	explicit Value(int32_t i32);
	/** @brief 64位无符号整数构造函数 */
	explicit Value(uint64_t u64);
	/** @brief 32位无符号整数构造函数 */
	explicit Value(uint32_t u32);

	// explicit Value(const UpValue& up_value);

	// explicit Value(ClassDef* class_def);
	/** @brief 模块定义构造函数 */
	explicit Value(ModuleDef* module_def);
	/** @brief 函数定义构造函数 */
	explicit Value(FunctionDef* function_def);
	/** @brief C++ 函数构造函数 */
	explicit Value(CppFunction bridge);
	/** @brief 导出变量构造函数 */
	explicit Value(ExportVar* export_var);
	/** @brief 闭包变量构造函数 */
	explicit Value(ClosureVar* closure_var);

	/** @brief 指定类型构造函数 */
	Value(ValueType type);
	/** @brief 指定类型的 Promise 对象构造函数 */
	Value(ValueType type, PromiseObject* promise);
	// Value(ValueType type, ClassDef* class_def);

	/** @brief 析构函数 */
	~Value();


	/** @brief 拷贝构造函数 */
	Value(const Value& r);
	/** @brief 移动构造函数 */
	Value(Value&& r) noexcept;

	/** @brief 拷贝赋值运算符 */
	void operator=(const Value& r);
	/** @brief 移动赋值运算符 */
	void operator=(Value&& r) noexcept;

	/**
	 * @brief 相等比较运算符
	 * @param r 要比较的值
	 * @return 是否相等
	 * @note 使用默认比较器，不处理类型转换
	 */
	bool operator==(const Value& r) const {
		return Comparer(nullptr, r) == 0;
	}

	/**
	 * @brief 比较器函数
	 * @param context 执行上下文指针
	 * @param rhs 要比较的右值
	 * @return 比较结果：负数表示小于，0表示等于，正数表示大于
	 */
	ptrdiff_t Comparer(Context* context, const Value& rhs) const;
	/** @brief 小于比较 */
	Value LessThan(Context* context, const Value& rhs) const;
	/** @brief 小于等于比较 */
	Value LessThanOrEqual(Context* context, const Value& rhs) const;
	/** @brief 大于比较 */
	Value GreaterThan(Context* context, const Value& rhs) const;
	/** @brief 大于等于比较 */
	Value GreaterThanOrEqual(Context* context, const Value& rhs) const;
	/** @brief 不等于比较 */
	Value NotEqualTo(Context* context, const Value& rhs) const;
	/** @brief 等于比较 */
	Value EqualTo(Context* context, const Value& rhs) const;

	/** @brief 加法运算 */
	Value Add(Context* context, const Value& rhs) const;
	/** @brief 减法运算 */
	Value Subtract(Context* context, const Value& rhs) const;
	/** @brief 乘法运算 */
	Value Multiply(Context* context, const Value& rhs) const;
	/** @brief 除法运算 */
	Value Divide(Context* context, const Value& rhs) const;
	/** @brief 左移位运算 */
	Value LeftShift(Context* context, const Value& rhs) const;
	/** @brief 右移位运算 */
	Value RightShift(Context* context, const Value& rhs) const;
	/** @brief 无符号右移位运算 */
	Value UnsignedRightShift(Context* context, const Value& rhs) const;
	/** @brief 按位与运算 */
	Value BitwiseAnd(Context* context, const Value& rhs) const;
	/** @brief 按位或运算 */
	Value BitwiseOr(Context* context, const Value& rhs) const;
	/** @brief 按位异或运算 */
	Value BitwiseXor(Context* context, const Value& rhs) const;
	/** @brief 按位取反运算 */
	Value BitwiseNot(Context* context) const;
	/** @brief 取反运算 */
	Value Negate(Context* context) const;
	/** @brief 前缀递增运算 */
	Value Increment(Context* context);
	/** @brief 前缀递减运算 */
	Value Decrement(Context* context);
	/** @brief 后缀递增运算 */
	Value PostIncrement(Context* context);
	/** @brief 后缀递减运算 */
	Value PostDecrement(Context* context);

	/** @brief 获取值类型 */
	ValueType type() const;

	/** @brief 获取布尔值 */
	bool boolean() const;
	/** @brief 设置布尔值 */
	void set_boolean(bool boolean);
	/** @brief 获取字符串视图 */
	const char* string_view() const;
	/** @brief 获取字符串引用 */
	const String& string() const;
	/** @brief 获取符号引用 */
	const Symbol& symbol() const;

	/** @brief 获取64位浮点数值 */
	double f64() const;
	/** @brief 设置64位浮点数值 */
	void set_float64(double number);
	/** @brief 获取64位整数值 */
	int64_t i64() const;
	/** @brief 获取64位无符号整数值 */
	uint64_t u64() const;

	/** @brief 获取对象引用 */
	Object& object() const;
	/**
	 * @brief 获取指定类型的对象引用
	 * @tparam ObjectT 对象类型
	 * @return 指定类型的对象引用
	 */
	template<typename ObjectT>
	ObjectT& object() const {
		return static_cast<ObjectT&>(object());
	}
	/** @brief 获取数组对象引用 */
	ArrayObject& array() const;
	/** @brief 获取函数对象引用 */
	FunctionObject& function() const;
	/** @brief 获取生成器对象引用 */
	GeneratorObject& generator() const;
	/** @brief 获取 Promise 对象引用 */
	PromiseObject& promise() const;
	/** @brief 获取异步对象引用 */
	AsyncObject& async() const;
	/** @brief 获取 C++ 模块对象引用 */
	CppModuleObject& cpp_module() const;
	/** @brief 获取模块对象引用 */
	ModuleObject& module() const;
	/** @brief 获取构造函数对象引用 */
	ConstructorObject& constructor() const;

	// ClassDef& class_def() const;
	/** @brief 获取模块定义引用 */
	ModuleDef& module_def() const;
	/** @brief 获取函数定义引用 */
	FunctionDef& function_def() const;
	/** @brief 获取 C++ 函数指针 */
	CppFunction cpp_function() const;
	/** @brief 获取导出变量引用 */
	ExportVar& export_var() const;
	/** @brief 获取闭包变量引用 */
	ClosureVar& closure_var() const;

	/** @brief 获取常量索引 */
	ConstIndex const_index() const { return tag_.const_index_; }
	/** @brief 设置常量索引 */
	void set_const_index(ConstIndex const_index) { tag_.const_index_ = const_index; }

	/** @brief 计算哈希值 */
	size_t hash() const;

	/** @brief 检查是否为 undefined 类型 */
	bool IsUndefined() const;
	/** @brief 检查是否为 null 类型 */
	bool IsNull() const;
	/** @brief 检查是否为 boolean 类型 */
	bool IsBoolean() const;
	/** @brief 检查是否为 number 类型 */
	bool IsNumber() const;
	/** @brief 检查是否为 string 类型 */
	bool IsString() const;
	/** @brief 检查是否为 string_view 类型 */
	bool IsStringView() const;
	/** @brief 检查是否为 symbol 类型 */
	bool IsSymbol() const;

	/** @brief 检查是否为引用计数类型 */
	bool IsReferenceCounter() const;
	/** @brief 增加引用计数 */
	void ReferenceCounterInc();
	/** @brief 减少引用计数 */
	void ReferenceCounterDec();


	/**
	 * @brief 检查是否为对象类型
	 * @note 新对象类型必须添加到 IsObject() 方法中，否则会导致内存泄漏
	 */
	bool IsObject() const;
	/** @brief 检查是否为数组对象类型 */
	bool IsArrayObject() const;
	/** @brief 检查是否为函数对象类型 */
	bool IsFunctionObject() const;
	/** @brief 检查是否为生成器对象类型 */
	bool IsGeneratorObject() const;
	/** @brief 检查是否为 Promise 对象类型 */
	bool IsPromiseObject() const;
	/** @brief 检查是否为异步对象类型 */
	bool IsAsyncObject() const;
	/** @brief 检查是否为异步解析恢复类型 */
	bool IsAsyncResolveResume() const;
	/** @brief 检查是否为异步拒绝恢复类型 */
	bool IsAsyncRejectResume() const;
	/** @brief 检查是否为 C++ 模块对象类型 */
	bool IsCppModuleObject() const;
	/** @brief 检查是否为模块对象类型 */
	bool IsModuleObject() const;
	/** @brief 检查是否为构造函数对象类型 */
	bool IsConstructorObject() const;
	/** @brief 检查是否为 Promise 解析类型 */
	bool IsPromiseResolve() const;
	/** @brief 检查是否为 Promise 拒绝类型 */
	bool IsPromiseReject() const;

	/** @brief 检查是否为浮点数类型 */
	bool IsFloat() const;
	/** @brief 检查是否为64位整数类型 */
	bool IsInt64() const;
	/** @brief 检查是否为64位无符号整数类型 */
	bool IsUInt64() const;

	// bool IsClassDef() const;
	/** @brief 检查是否为模块定义类型 */
	bool IsModuleDef() const;
	/** @brief 检查是否为函数定义类型 */
	bool IsFunctionDef() const;
	/** @brief 检查是否为 C++ 函数类型 */
	bool IsCppFunction() const;
	/** @brief 检查是否为导出变量类型 */
	bool IsExportVar() const;
	/** @brief 检查是否为闭包变量类型 */
	bool IsClosureVar() const;

	/** @brief 检查是否为生成器下一个值类型 */
	bool IsGeneratorNext() const;
	/** @brief 检查是否为迭代器对象类型 */
	bool IsIteratorObject() const;

	/** @brief 转换为字符串表示 */
	Value ToString(Context* context) const;
	/** @brief 转换为布尔值 */
	Value ToBoolean() const;
	/** @brief 转换为数字值 */
	Value ToNumber() const;
	/** @brief 转换为64位整数值 */
	Value ToInt64() const;
	/** @brief 转换为64位无符号整数值 */
	Value ToUInt64() const;
	/** @brief 转换为模块定义引用 */
	const ModuleDef& ToModuleDef() const;
	/** @brief 转换为函数定义引用 */
	const FunctionDef& ToFunctionDef() const;

	/** @brief 检查是否为异常返回 */
	bool IsException() const { return tag_.exception_; }
	/** @brief 设置为异常返回 */
	Value& SetException() { tag_.exception_ = 1; return *this; }

	/** @brief 获取对象属性 */
	bool GetProperty(Context* context, ConstIndex key, Value* value);

	/**
	 * @brief 将值类型转换为字符串表示
	 * @param type 值类型
	 * @return 类型名称字符串
	 * @throw std::runtime_error 当类型无效时抛出
	 */
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
			return "object";
		case ValueType::kFloatObject:
			return "float_object";
		case ValueType::kStringObject:
			return "string_object";
		case ValueType::kArrayObject:
			return "array_object";
		case ValueType::kFunctionObject:
			return "function_object";
		case ValueType::kGeneratorObject:
			return "generator_object";
		case ValueType::kPromiseObject:
			return "promise_object";
		case ValueType::kAsyncObject:
			return "async_object";
		case ValueType::kCppModuleObject:
			return "cpp_module_object";
		case ValueType::kModuleObject:
			return "module_object";
		case ValueType::kConstructorObject:
			return "constructor_object";
		case ValueType::kFunctionDef:
			return "function_def";
		case ValueType::kCppFunction:
			return "cpp_function";
		case ValueType::kClosureVar:
			return "closure_var";
		case ValueType::kModuleDef:
			return "module_def";
		case ValueType::kExportVar:
			return "export_var";
		case ValueType::kGeneratorNext:
			return "generator_next";
		case ValueType::kAsyncResolveResume:
			return "async_resolve_resume";
		case ValueType::kAsyncRejectResume:
			return "async_reject_resume";
		case ValueType::kPromiseResolve:
			return "promise_resolve";
		case ValueType::kPromiseReject:
			return "promise_reject";
		default:
			return "unknown_type";
		}
	}

private:
	/** @brief 清空值内容 */
	void Clear();
	/** @brief 拷贝值内容 */
	void Copy(const Value& r);
	/** @brief 移动值内容 */
	void Move(Value&& r);

private:
	/** @brief 值标签联合体 */
	union {
		uint64_t full_ = 0;                    ///< 完整64位值
		struct {
			ValueType type_ : 16;                ///< 值类型（16位）
			uint32_t exception_ : 1;             ///< 是否为异常返回标记
			ConstIndex const_index_;             ///< 常量索引（非0表示来自常量池）
		};
	} tag_;
	/** @brief 值数据联合体 */
	union {
		uint64_t full_ = 0;                    ///< 完整64位值

		bool boolean_;                         ///< 布尔值
		double f64_;                           ///< 64位浮点数
		String* string_;                       ///< 字符串指针
		Symbol* symbol_;                       ///< 符号指针

		Object* object_;                       ///< 对象指针

		int64_t i64_;                          ///< 64位整数
		uint64_t u64_;                         ///< 64位无符号整数
		const char* string_view_;              ///< 字符串视图指针

		ClassDef* class_def_;                  ///< 类定义指针

		ModuleDef* module_def_;                ///< 模块定义指针
		FunctionDef* function_def_;            ///< 函数定义指针
		CppFunction cpp_func_;                 ///< C++ 函数指针
		ExportVar* export_var_;                ///< 导出变量指针
		ClosureVar* closure_var_;              ///< 闭包变量指针

		ExceptionIdx exception_idx_;           ///< 异常索引
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
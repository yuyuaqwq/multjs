#include <mjs/class_def/array_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/object/array_object.h>
#include <mjs/value/object/function_object.h>

namespace mjs {

ArrayObjectClassDef::ArrayObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArrayObject, "Array")
{
	// Array.prototype.__proto__ = Object.prototype
	prototype_.object().SetPrototype(&runtime->default_context(), runtime->class_def_table()[ClassId::kObject].prototype());
	// Array.__proto = Function.prototype
	constructor_.object().SetPrototype(&runtime->default_context(), runtime->class_def_table()[ClassId::kFunctionObject].prototype());

	constructor_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kOf, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		return ArrayObjectClassDef::Of(context, par_count, stack);
	}));

	// Push method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kPush, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& arr = stack.this_val().array();
		for (size_t i = 0; i < par_count; ++i) {
			arr.Push(context, stack.get(i));
		}
		return Value(static_cast<int64_t>(arr.GetLength()));
	}));

	// Pop method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kPop, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& arr = stack.this_val().array();
		if (arr.GetLength() == 0) {
			return Value();
		}
		return arr.Pop(context);
	}));

	// ForEach method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kForEach, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "forEach requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject() && !callback.IsFunctionDef()) {
			return TypeError::Throw(context, "forEach callback must be a function");
		}

		for (size_t i = 0; i < arr.GetLength(); ++i) {
			Value elem;
			bool exists = arr.GetComputedProperty(context, Value(static_cast<int64_t>(i)), &elem);
			// JS标准：跳过空洞元素（不存在的属性）
			if (!exists) {
				continue;
			}
			std::array<Value, 3> args = {
				elem,
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			context->CallFunction(&callback, Value(), args.begin(), args.end());
		}
		return Value();
	}));

	// Map method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kMap, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "map requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject() && !callback.IsFunctionDef()) {
			return TypeError::Throw(context, "map callback must be a function");
		}

		GCHandleScope<1> scope(context);
		auto result = scope.New<ArrayObject>(arr.GetLength());
		for (size_t i = 0; i < arr.GetLength(); ++i) {
			Value elem;
			bool exists = arr.GetComputedProperty(context, Value(static_cast<int64_t>(i)), &elem);
			// JS标准：跳过空洞元素，但保持索引位置（创建稀疏数组）
			if (!exists) {
				continue;
			}
			std::array<Value, 3> args = {
				elem,
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			auto mapped_value = context->CallFunction(&callback, Value(), args.begin(), args.end());
			result->SetComputedProperty(context, Value(static_cast<int64_t>(i)), std::move(mapped_value));
		}
		return scope.Close(result);
	}));

	// Filter method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kFilter, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "filter requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject()) {
			return TypeError::Throw(context, "filter callback must be a function");
		}

		GCHandleScope<1> scope(context);
		auto result = scope.New<ArrayObject>();
		for (size_t i = 0; i < arr.GetLength(); ++i) {
			Value elem;
			bool exists = arr.GetComputedProperty(context, Value(static_cast<int64_t>(i)), &elem);
			// JS标准：跳过空洞元素
			if (!exists) {
				continue;
			}
			std::array<Value, 3> args = {
				elem,
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			auto callback_result = context->CallFunction(&callback, Value(), args.begin(), args.end());
			if (callback_result.ToBoolean().boolean()) {
				result->Push(context, elem);
			}
		}
		return scope.Close(result);
	}));

	// Reduce method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kReduce, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "reduce requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject() && !callback.IsFunctionDef()) {
			return TypeError::Throw(context, "reduce callback must be a function");
		}

		size_t start_index = 0;
		Value accumulator;

		if (par_count >= 2) {
			// 提供了初始值
			accumulator = stack.get(1);
		} else {
			// 没有提供初始值，使用数组第一个非空洞元素作为初始值
			bool found = false;
			for (size_t i = 0; i < arr.GetLength(); ++i) {
				Value elem;
				if (arr.GetComputedProperty(context, Value(static_cast<int64_t>(i)), &elem)) {
					accumulator = elem;
					start_index = i + 1;
					found = true;
					break;
				}
			}
			if (!found) {
				return TypeError::Throw(context, "reduce of empty array with no initial value");
			}
		}

		for (size_t i = start_index; i < arr.GetLength(); ++i) {
			Value elem;
			bool exists = arr.GetComputedProperty(context, Value(static_cast<int64_t>(i)), &elem);
			// JS标准：跳过空洞元素
			if (!exists) {
				continue;
			}
			std::array<Value, 4> args = {
				accumulator,
				elem,
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			accumulator = context->CallFunction(&callback, Value(), args.begin(), args.end());
		}
		return accumulator;
	}));
}

Value ArrayObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
	// 标准 new Array() 行为:
	// - new Array() -> []
	// - new Array(3) -> [empty x 3] (单个数字参数)
	// - new Array(1, 2, 3) -> [1, 2, 3] (多个参数)

	if (par_count == 0) {
		// new Array()
		GCHandleScope<1> scope(context);
		auto arr = scope.New<ArrayObject>(0);
		return scope.Close(arr);
	}
	else if (par_count == 1) {
		// new Array(x)
		auto& arg = stack.get(0);
		if (arg.type() == ValueType::kInt64 || arg.type() == ValueType::kFloat64) {
			// 数字参数: 创建指定长度的数组
			uint64_t len;
			if (arg.type() == ValueType::kInt64) {
				if (arg.i64() < 0) {
					return RangeError::Throw(context, "Invalid array length");
				}
				len = static_cast<uint64_t>(arg.i64());
			} else {
				auto fval = arg.f64();
				if (fval < 0 || !std::isfinite(fval)) {
					return RangeError::Throw(context, "Invalid array length");
				}
				len = static_cast<uint64_t>(fval);
			}
			GCHandleScope<1> scope(context);
			auto arr = scope.New<ArrayObject>(len);
			return scope.Close(arr);
		} else {
			// 非数字参数: 创建包含该元素的数组
			GCHandleScope<1> scope(context);
			auto arr = scope.New<ArrayObject>(1);
			arr->SetComputedProperty(context, Value(static_cast<int64_t>(0)), std::move(arg));
			return scope.Close(arr);
		}
	} else {
		// 多个参数: new Array(1, 2, 3)
		return LiteralNew(context, par_count, stack);
	}
}

Value ArrayObjectClassDef::Of(Context* context, uint32_t par_count, const StackFrame& stack) {
	return LiteralNew(context, par_count, stack);
}

Value ArrayObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	GCHandleScope<1> scope(context);
	auto arr = scope.New<ArrayObject>(par_count);
	for (size_t i = 0; i < par_count; ++i) {
		arr->SetComputedProperty(context, Value(static_cast<int64_t>(i)), std::move(stack.get(i)));
	}
	return scope.Close(arr);
}

} // namespace mjs
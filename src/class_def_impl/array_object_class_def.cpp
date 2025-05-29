#include <mjs/class_def_impl/array_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object_impl/array_object.h>
#include <mjs/object_impl/function_object.h>

namespace mjs {

ArrayObjectClassDef::ArrayObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArrayObject, "Array")
{
	length_const_index_ = runtime->const_pool().insert(Value("length"));
	of_const_index_ = runtime->const_pool().insert(Value("of"));
	push_const_index_ = runtime->const_pool().insert(Value("push"));
	pop_const_index_ = runtime->const_pool().insert(Value("pop"));
	forEach_const_index_ = runtime->const_pool().insert(Value("forEach"));
	map_const_index_ = runtime->const_pool().insert(Value("map"));
	filter_const_index_ = runtime->const_pool().insert(Value("filter"));

	constructor_object_.object().SetProperty(nullptr, of_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		return ArrayObjectClassDef::Of(context, par_count, stack);
	}));

	// Push method
	prototype_.object().SetProperty(nullptr, push_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& arr = stack.this_val().array();
		for (size_t i = 0; i < par_count; ++i) {
			arr.Push(context, stack.get(i));
		}
		return Value(static_cast<int64_t>(arr.length()));
	}));

	// Pop method
	prototype_.object().SetProperty(nullptr, pop_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& arr = stack.this_val().array();
		if (arr.length() == 0) {
			return Value();
		}
		return arr.Pop(context);
	}));

	// ForEach method
	prototype_.object().SetProperty(nullptr, forEach_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "forEach requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject() && !callback.IsFunctionDef()) {
			return TypeError::Throw(context, "forEach callback must be a function");
		}

		for (size_t i = 0; i < arr.length(); ++i) {
			std::array<Value, 3> args = {
				arr[i],
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			context->CallFunction(&callback, Value(), args.begin(), args.end());
		}
		return Value();
	}));

	// Map method
	prototype_.object().SetProperty(nullptr, map_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "map requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject() && !callback.IsFunctionDef()) {
			return TypeError::Throw(context, "forEach callback must be a function");
		}

		auto result = ArrayObject::New(context, arr.length());
		for (size_t i = 0; i < arr.length(); ++i) {
			std::array<Value, 3> args = {
				arr[i],
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			context->CallFunction(&(*result)[i], Value(), args.begin(), args.end());
		}
		return Value(result);
	}));

	// Filter method
	prototype_.object().SetProperty(nullptr, filter_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return TypeError::Throw(context, "filter requires a callback function");
		}
		auto& arr = stack.this_val().array();
		auto callback = stack.get(0);
		if (!callback.IsFunctionObject()) {
			return TypeError::Throw(context, "filter callback must be a function");
		}

		auto result = ArrayObject::New(context, 0);
		for (size_t i = 0; i < arr.length(); ++i) {
			std::array<Value, 3> args = {
				arr[i],
				Value(static_cast<int64_t>(i)),
				stack.this_val()
			};
			auto callback_result = context->CallFunction(&(*result)[i], Value(), args.begin(), args.end());
			if (callback_result.ToBoolean().boolean()) {
				result->Push(context, arr[i]);
			}
		}
		return Value(result);
	}));
}

Value ArrayObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
	return Of(context, par_count, stack);
}

Value ArrayObjectClassDef::Of(Context* context, uint32_t par_count, const StackFrame& stack) {
	return LiteralNew(context, par_count, stack);
}

Value ArrayObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto arr = ArrayObject::New(context, par_count);
	for (size_t i = 0; i < par_count; ++i) {
		arr->operator[](i) = std::move(stack.get(i));
	}
	return Value(arr);
}

} // namespace mjs
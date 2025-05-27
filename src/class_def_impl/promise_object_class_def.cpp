#include <mjs/class_def_impl/promise_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object_impl/promise_object.h>

namespace mjs {

PromiseObjectClassDef::PromiseObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kPromiseObject, "Promise")
{
	prototype_.object().SetProperty(runtime, "then", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& promise = stack.this_val().promise();
		Value on_fulfilled;
		Value on_rejected;
		if (par_count > 0) {
			on_fulfilled = stack.get(0);
		}
		if (par_count > 1) {
			on_rejected = stack.get(1);
		}
		return promise.Then(context, on_fulfilled, on_rejected);
	}));

	constructor_object_.object().SetProperty(runtime, "resolve", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		Value result;
		if (par_count > 0) {
			result = stack.get(0);
		}
		return Resolve(context, std::move(result));
	}));

	constructor_object_.object().SetProperty(runtime, "reject", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		Value reason;
		if (par_count > 0) {
			reason = stack.get(0);
		}
		return Reject(context, std::move(reason));
	}));
}

Value PromiseObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	Value executor;
	if (par_count > 0) {
		executor = stack.get(0);
	}
	return Value(PromiseObject::New(context, std::move(executor)));
}

Value PromiseObjectClassDef::Resolve(Context* context, Value result) {
	auto promise = PromiseObject::New(context, Value());
	promise->Resolve(context, result);
	return Value(promise);
}

Value PromiseObjectClassDef::Reject(Context* context, Value reason) {
	auto promise = PromiseObject::New(context, Value());
	promise->Reject(context, reason);
	return Value(promise);
}

} // namespace mjs
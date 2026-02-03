#include <mjs/class_def/promise_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/object/promise_object.h>

namespace mjs {

PromiseObjectClassDef::PromiseObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kPromiseObject, "Promise")
{
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kThen, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
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

	constructor_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kResolve, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		Value result;
		if (par_count > 0) {
			result = stack.get(0);
		}
		return Resolve(context, std::move(result));
	}));

	constructor_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kReject, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		Value reason;
		if (par_count > 0) {
			reason = stack.get(0);
		}
		return Reject(context, std::move(reason));
	}));
}

Value PromiseObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
	Value executor;
	if (par_count > 0) {
		executor = stack.get(0);
	}
	GCHandleScope<1> scope(context);
	auto promise = scope.New<PromiseObject>(std::move(executor));
	return scope.Close(promise);
}

Value PromiseObjectClassDef::Resolve(Context* context, Value result) {
	GCHandleScope<1> scope(context);
	auto promise = scope.New<PromiseObject>(Value());
	promise->Resolve(context, result);
	return scope.Close(promise);
}

Value PromiseObjectClassDef::Reject(Context* context, Value reason) {
	GCHandleScope<1> scope(context);
	auto promise = scope.New<PromiseObject>(Value());
	promise->Reject(context, reason);
	return scope.Close(promise);
}

} // namespace mjs
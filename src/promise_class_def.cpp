#include <mjs/promise_class_def.h>

#include <mjs/promise_object.h>
#include <mjs/stack_frame.h>

namespace mjs {

PromiseClassDef::PromiseClassDef()
	: ClassDef(ClassId::kPromise, "Promise")
{
	property_map_.NewMethod(Value("resolve"), Value([](Context* context, const Value& this_val, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& promise = this_val.promise();
		promise.Resolve(context);
		return Value();
	}));
	property_map_.NewMethod(Value("reject"), Value([](Context* context, const Value& this_val, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& promise = this_val.promise();
		promise.Reject(context);
		return Value();
	}));
	property_map_.NewMethod(Value("then"), Value([](Context* context, const Value& this_val, uint32_t par_count, const StackFrame& stack) -> Value {
		auto& promise = this_val.promise();
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
}

Value PromiseClassDef::Constructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	Value resolve_func;
	Value reject_func;
	if (par_count > 0) {
		resolve_func = stack.get(0);
	}
	if (par_count > 1) {
		reject_func = stack.get(1);
	}
	return Value(new PromiseObject(context, std::move(resolve_func), std::move(reject_func)));
}

} // namespace mjs
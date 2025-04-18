#include <mjs/class_def/promise_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object/promise_object.h>

namespace mjs {

PromiseClassDef::PromiseClassDef()
	: ClassDef(ClassId::kPromise, "Promise")
{
	property_map_.NewMethod(Value("then"), Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
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

	static_property_map_.NewMethod(Value("resolve"), Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		Value value;
		if (par_count > 0) {
			value = stack.get(0);
		}
		return Resolve(context, std::move(value));
	}));

	static_property_map_.NewMethod(Value("reject"), Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		return Value();
	}));
}

Value PromiseClassDef::Constructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	Value executor;
	if (par_count > 0) {
		executor = stack.get(0);
	}
	return Value(new PromiseObject(context, std::move(executor)));
}

Value PromiseClassDef::Resolve(Context* context, Value value) {
	auto promise = new PromiseObject(context, Value());
	promise->Resolve(context, value);
	return Value(promise);
}

} // namespace mjs
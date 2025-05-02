#include <mjs/class_def/array_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object/array_object.h>

namespace mjs {

ArrayClassDef::ArrayClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArray, "Array")
{
	of_const_index_ = runtime->const_pool().insert(Value(String::make("of")));

	static_property_map_.set(runtime, of_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		return ArrayClassDef::Of(context, par_count, stack);
	}));
}

Value ArrayClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	return Of(context, par_count, stack);
}

Value ArrayClassDef::Of(Context* context, uint32_t par_count, const StackFrame& stack) {
	return LiteralNew(context, par_count, stack);
}

Value ArrayClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto arr = new ArrayObject(context, par_count);
	for (size_t i = 0; i < par_count; ++i) {
		arr->operator[](i) = std::move(stack.get(i));
	}
	return Value(arr);
}

} // namespace mjs
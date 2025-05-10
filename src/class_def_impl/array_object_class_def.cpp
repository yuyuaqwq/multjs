#include <mjs/class_def_impl/array_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object_impl/array_object.h>

namespace mjs {

ArrayObjectClassDef::ArrayObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArrayObject, "Array")
{
	length_const_index_ = runtime->const_pool().insert(Value("length"));
	of_const_index_ = runtime->const_pool().insert(Value("of"));

	constructor_object_.object().SetProperty(nullptr, of_const_index_, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		return ArrayObjectClassDef::Of(context, par_count, stack);
	}));
}

Value ArrayObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	return Of(context, par_count, stack);
}

bool ArrayObjectClassDef::GetProperty(Context* context, Object* obj, ConstIndex key, Value* value) {
	if (key == length_const_index_) {
		*value = Value(obj->get<ArrayObject>().length());
		return true;
	}
	return ClassDef::GetProperty(context, obj, key, value);
}

Value ArrayObjectClassDef::Of(Context* context, uint32_t par_count, const StackFrame& stack) {
	return LiteralNew(context, par_count, stack);
}

Value ArrayObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto arr = new ArrayObject(context, par_count);
	for (size_t i = 0; i < par_count; ++i) {
		arr->operator[](i) = std::move(stack.get(i));
	}
	return Value(arr);
}

} // namespace mjs
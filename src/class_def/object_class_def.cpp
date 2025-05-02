#include <mjs/class_def/object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object/array_object.h>

namespace mjs {

ObjectClassDef::ObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kObject, "Object") {}

Value ObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
	return Value(new Object(context));
}

Value ObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto obj = new Object(context);
	for (size_t i = 0; i < par_count - 1; i += 2) {
		auto key_const_index = stack.get(i).const_index();
		assert(!key_const_index.is_invalid());
		obj->SetProperty(context, key_const_index, std::move(stack.get(i + 1)));
	}
	return Value(obj);
}

} // namespace mjs
#include <mjs/class_def/iterator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

IteratorClassDef::IteratorClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArrayObject, "Iterator")
{
	next_const_index_ = runtime->const_pool().insert(Value(String::make("next")));
}


} // namespace mjs
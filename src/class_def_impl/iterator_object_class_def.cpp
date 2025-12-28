#include <mjs/class_def_impl/iterator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

IteratorClassDef::IteratorClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kArrayObject, "Iterator")
{
	next_const_index_ = runtime->global_const_pool().insert(Value("next"));
}


} // namespace mjs
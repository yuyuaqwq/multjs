#include <mjs/class_def_impl/generator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

GeneratorObjectClassDef::GeneratorObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kGeneratorObject, "Generator")
{
	value_const_idx_ = runtime->const_pool().insert(Value("value"));
	done_const_idx_ = runtime->const_pool().insert(Value("done"));

	property_map_.insert(runtime, "next", Value(ValueType::kGeneratorNext));
}

} // namespace mjs
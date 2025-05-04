#include <mjs/class_def_impl/generator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

GeneratorObjectClassDef::GeneratorObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kGeneratorObject, "Generator")
{
	value_const_idx_ = runtime->const_pool().insert(Value(String::make("value")));
	done_const_idx_ = runtime->const_pool().insert(Value(String::make("done")));

	property_map_.insert(runtime, String::make("next"), Value(ValueType::kGeneratorNext));
}

} // namespace mjs
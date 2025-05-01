#include <mjs/class_def/generator_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

GeneratorClassDef::GeneratorClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kGenerator, "Generator")
{
	value_const_idx_ = runtime->const_pool().insert(Value(String::make("value")));
	done_const_idx_ = runtime->const_pool().insert(Value(String::make("done")));

	property_map_.emplace(runtime, String::make("next"), Value(ValueType::kGeneratorNext));
}

} // namespace mjs
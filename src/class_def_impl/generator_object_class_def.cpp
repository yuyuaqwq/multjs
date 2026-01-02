#include <mjs/class_def_impl/generator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

GeneratorObjectClassDef::GeneratorObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kGeneratorObject, "Generator")
{
	value_const_index_ = runtime->global_const_pool().insert(Value("value"));
	done_const_index_ = runtime->global_const_pool().insert(Value("done"));

	auto next_const_index = runtime->global_const_pool().insert(Value("next"));

	prototype_.object().SetProperty(runtime, next_const_index, Value(ValueType::kGeneratorNext));
}

} // namespace mjs
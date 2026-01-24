#include <mjs/class_def_impl/generator_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

GeneratorObjectClassDef::GeneratorObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kGeneratorObject, nullptr)
{
	prototype_.object().SetProperty(runtime, ConstIndexEmbedded::kNext, Value(ValueType::kGeneratorNext));
}

} // namespace mjs
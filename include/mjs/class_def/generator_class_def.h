#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef(Runtime* runtime)
		: ClassDef(runtime, ClassId::kGenerator, "Generator")
	{
		// key到时候优化到runtime的const pool里
		property_map_.NewMethod(runtime, "next", Value(ValueType::kGeneratorNext));
	}
};

} // namespace mjs
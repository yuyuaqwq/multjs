#pragma once

#include <mjs/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef()
		: ClassDef(ClassId::kGenerator, "Generator")
	{
		// key到时候优化到runtime的const pool里
		property_map_.NewMethod(Value("next"), Value(ValueType::kGeneratorNext));
	}
};

} // namespace mjs
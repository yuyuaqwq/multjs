#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef(Runtime* runtime)
		: ClassDef(runtime, ClassId::kGenerator, "Generator")
	{
		// key��ʱ���Ż���runtime��const pool��
		property_map_.NewMethod(runtime, "next", Value(ValueType::kGeneratorNext));
	}
};

} // namespace mjs
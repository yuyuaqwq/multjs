#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef()
		: ClassDef(ClassId::kGenerator, "Generator")
	{
		// key��ʱ���Ż���runtime��const pool��
		property_map_.NewMethod(Value("next"), Value(ValueType::kGeneratorNext));
	}
};

} // namespace mjs
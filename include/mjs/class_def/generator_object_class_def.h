#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorObjectClassDef : public ClassDef {
public:
	GeneratorObjectClassDef(Runtime* runtime);
};

} // namespace mjs
#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class FunctionObjectClassDef : public ClassDef {
public:
	FunctionObjectClassDef(Runtime* runtime);
};

} // namespace mjs
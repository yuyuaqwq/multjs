#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class StringObjectClassDef : public ClassDef {
public:
	StringObjectClassDef(Runtime* runtime);
};

} // namespace mjs
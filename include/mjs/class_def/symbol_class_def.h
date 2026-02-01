#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class SymbolClassDef : public ClassDef {
public:
	SymbolClassDef(Runtime* runtime);

	static Value For(Context* context, Value name);
};

} // namespace mjs
#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class SymbolClassDef : public ClassDef {
public:
	SymbolClassDef(Runtime* runtime);

	Value For(Context* context, ConstIndex idx);

private:

};

} // namespace mjs
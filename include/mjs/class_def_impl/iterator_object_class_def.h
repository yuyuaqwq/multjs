#pragma once

#include <mjs/class_def.h>

namespace mjs {

class IteratorClassDef : public ClassDef {
public:
	IteratorClassDef(Runtime* runtime);

	virtual void Next();
};

} // namespace mjs
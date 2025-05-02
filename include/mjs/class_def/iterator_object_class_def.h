#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class IteratorClassDef : public ClassDef {
public:
	IteratorClassDef(Runtime* runtime);

	virtual void Next();

private:
	ConstIndex next_const_index_;
};

} // namespace mjs
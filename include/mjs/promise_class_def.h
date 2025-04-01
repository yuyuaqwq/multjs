#pragma once

#include <mjs/class_def.h>

namespace mjs {

class PromiseClassDef : public ClassDef {
public:
	PromiseClassDef();

	virtual Value Constructor(Context* context, uint32_t par_count, StackFrame* stack) override;
};

} // namespace mjs
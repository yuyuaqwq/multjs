#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class PromiseClassDef : public ClassDef {
public:
	PromiseClassDef(Runtime* runtime);

	Value Constructor(Context* context, uint32_t par_count, const StackFrame& stack) override;

	static Value Resolve(Context* context, Value value);
};

} // namespace mjs
#pragma once

#include <mjs/class_def.h>

namespace mjs {

class ArrayObjectClassDef : public ClassDef {
public:
	ArrayObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const override;

	static Value Of(Context* context, uint32_t par_count, const StackFrame& stack);

	static Value LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack);
};

} // namespace mjs
#pragma once

#include <mjs/class_def.h>

namespace mjs {

class ObjectClassDef : public ClassDef {
public:
	ObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) override;

	static Value LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack);

private:

};

} // namespace mjs
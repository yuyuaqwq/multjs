#pragma once

#include <mjs/class_def.h>

namespace mjs {

class PromiseObjectClassDef : public ClassDef {
public:
	PromiseObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) override;

	static Value Resolve(Context* context, Value value);
};

} // namespace mjs
#pragma once

#include <mjs/class_def.h>

namespace mjs {

class PromiseObjectClassDef : public ClassDef {
public:
	PromiseObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const override;

	static Value Resolve(Context* context, Value result);

	static Value Reject(Context* context, Value reason);
};

} // namespace mjs
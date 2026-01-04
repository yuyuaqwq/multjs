#pragma once

#include <mjs/class_def.h>

namespace mjs {

class ObjectClassDef : public ClassDef {
public:
	ObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const override;

	static Value LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack);
	static Value SetProperty(Context* context, uint32_t par_count, const StackFrame& stack);
	static Value DefineProperty(Context* context, uint32_t par_count, const StackFrame& stack);
	static Value Freeze(Context* context, uint32_t par_count, const StackFrame& stack);
	static Value Seal(Context* context, uint32_t par_count, const StackFrame& stack);
	static Value PreventExtensions(Context* context, uint32_t par_count, const StackFrame& stack);

private:

};

} // namespace mjs
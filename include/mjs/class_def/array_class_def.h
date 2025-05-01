#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class ArrayClassDef : public ClassDef {
public:
	ArrayClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack);

	static Value Of(Context* context, uint32_t par_count, const StackFrame& stack);

	ConstIndex of_const_index() const { return of_const_index_; }

private:
	ConstIndex of_const_index_;
};

} // namespace mjs
#pragma once

#include <mjs/class_def.h>

namespace mjs {

class ArrayObjectClassDef : public ClassDef {
public:
	ArrayObjectClassDef(Runtime* runtime);

	Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) override;

	bool GetProperty(Context* context, Object* obj, ConstIndex key, Value* value) override;


	static Value Of(Context* context, uint32_t par_count, const StackFrame& stack);

	static Value LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack);


	ConstIndex of_const_index() const { return of_const_index_; }

private:
	ConstIndex length_const_index_;
	ConstIndex of_const_index_;
	ConstIndex push_const_index_;
	ConstIndex pop_const_index_;
	ConstIndex forEach_const_index_;
	ConstIndex map_const_index_;
	ConstIndex filter_const_index_;

};

} // namespace mjs
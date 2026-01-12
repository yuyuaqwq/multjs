#pragma once

#include <mjs/class_def.h>

namespace mjs {

class FunctionObjectClassDef : public ClassDef {
public:
	FunctionObjectClassDef(Runtime* runtime);

	ConstIndex prototype_const_index() const { return prototype_const_index_; }

	ConstIndex constructor_const_index() const { return constructor_const_index_; }

private:
	ConstIndex prototype_const_index_;
	ConstIndex constructor_const_index_;
};

} // namespace mjs
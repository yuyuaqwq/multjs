#pragma once

#include <mjs/class_def.h>

namespace mjs {

class GeneratorObjectClassDef : public ClassDef {
public:
	GeneratorObjectClassDef(Runtime* runtime);

	auto value_const_index() const { return value_const_index_; }
	
	auto done_const_index() const { return done_const_index_; }

private:
	ConstIndex value_const_index_;
	ConstIndex done_const_index_;
};

} // namespace mjs
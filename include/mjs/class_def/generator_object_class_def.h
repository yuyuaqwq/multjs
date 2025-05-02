#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorObjectClassDef : public ClassDef {
public:
	GeneratorObjectClassDef(Runtime* runtime);

	auto value_const_idx() const { return value_const_idx_; }
	auto done_const_idx() const { return done_const_idx_; }

private:
	ConstIndex value_const_idx_;
	ConstIndex done_const_idx_;

};

} // namespace mjs
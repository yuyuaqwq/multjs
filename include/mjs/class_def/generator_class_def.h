#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef(Runtime* runtime);

	auto value_const_idx() const { return value_const_idx_; }
	auto done_const_idx() const { return done_const_idx_; }

private:
	ConstIndex value_const_idx_;
	ConstIndex done_const_idx_;

};

} // namespace mjs
#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

class GeneratorClassDef : public ClassDef {
public:
	GeneratorClassDef(Runtime* runtime)
		: ClassDef(runtime, ClassId::kGenerator, "Generator")
	{
		property_map_.emplace(runtime, String::make("next"), Value(ValueType::kGeneratorNext));

		value_const_idx_ = runtime->const_pool().insert(Value("value"));
		done_const_idx_ = runtime->const_pool().insert(Value("done"));
	}

	auto value_const_idx() const { return value_const_idx_; }
	auto done_const_idx() const { return done_const_idx_; }

private:
	ConstIndex value_const_idx_;
	ConstIndex done_const_idx_;

};

} // namespace mjs
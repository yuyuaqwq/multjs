#include <mjs/up_value.h>

#include <mjs/value.h>

namespace mjs {

Value& UpValue::Up() const {
	Value* val = up_;
	while (val->IsUpValue()) {
		val = val->up_value().up_;
	}
	return *val;
}

} // namespace mjs
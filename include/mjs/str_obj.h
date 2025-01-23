#pragma once

#include <mjs/object.h>

namespace mjs {

class StringObject : public Object {
public:
	const char* str() const { return str_; }
	char* mutable_str() { return str_; }

private:
	char str_[8];
};

} // namespace mjs
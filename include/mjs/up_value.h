#pragma once

#include "string.h"

namespace mjs {

class Value;
class UpValue {
public:
	UpValue() = default;
	UpValue(Value* up) : up_(up) {}

	Value& Up() const;

private:
	Value* up_ = nullptr;
};

} // namespace mjs
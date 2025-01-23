#pragma once

#include <mjs/value.h>

namespace mjs {

class ArrayObject : public Object {
public:
    std::vector<Value>& mutale_values() { return values_; }

private:
    std::vector<Value> values_;
};

} // namespace mjs
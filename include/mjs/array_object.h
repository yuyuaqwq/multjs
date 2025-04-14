#pragma once

#include <mjs/object.h>

namespace mjs {

class ArrayObject : public Object {
public:
    using Object::Object;

    // std::vector<Value>& mutale_values() { return values_; }

private:
    // 优化项，优先从vector中查找，找不到才找prop_map
    // std::vector<Value> values_;
};

} // namespace mjs
#pragma once

#include <mjs/object.h>
#include <mjs/value.h>

namespace mjs {

class ArrayObject : public Object {
public:
    // std::vector<Value>& mutale_values() { return values_; }

private:
    // �Ż�����ȴ�vector�в��ң��Ҳ�������prop_map
    // std::vector<Value> values_;
};

} // namespace mjs
#pragma once

#include <mjs/object.h>
#include <mjs/value.h>

namespace mjs {

class GeneratorObject : public Object {
public:
    

private:
    // Éú³ÉÆ÷º¯Êý
    Value func_;

    int state_;
};

} // namespace mjs
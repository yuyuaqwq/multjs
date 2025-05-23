#pragma once

#include <mjs/object.h>

namespace mjs {

class NamespaceObject : public Object {
public:
    NamespaceObject(Context* context, size_t length)
        : Object(context, ClassId::kObject) {}
};

} // namespace mjs
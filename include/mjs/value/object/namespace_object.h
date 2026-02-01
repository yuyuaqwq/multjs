#pragma once

#include <mjs/object.h>

namespace mjs {

class NamespaceObject : public Object {
private:
    NamespaceObject(Context* context)
        : Object(context, ClassId::kObject) {}

public:
	static NamespaceObject* New(Context* context) {
		return new NamespaceObject(context);
	}
};

} // namespace mjs
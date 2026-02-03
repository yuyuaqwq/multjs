#pragma once

#include <mjs/value/object/object.h>

namespace mjs {

class CppModuleObject : public Object {
protected:
	CppModuleObject(Context* context);

public:
	static CppModuleObject* New(Context* context) {
		return nullptr;
    }
};

} // namespace mjs
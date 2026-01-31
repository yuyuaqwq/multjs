#pragma once

#include <mjs/object.h>

namespace mjs {

class CppModuleObject : public Object {
protected:
	CppModuleObject(Context* context);

public:
	static CppModuleObject* New(Context* context) {
		return new CppModuleObject(context);
    }
};

} // namespace mjs
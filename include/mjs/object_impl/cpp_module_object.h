#pragma once

#include <mjs/object.h>

namespace mjs {

class CppModuleObject : public Object {
protected:
	CppModuleObject(Runtime* runtime);

public:
	void AddExportMethod(Runtime* runtime, std::string_view name, CppFunction function);

	static CppModuleObject* New(Runtime* runtime) {
		return new CppModuleObject(runtime);
    }
};

} // namespace mjs
#pragma once

#include <mjs/object.h>

namespace mjs {

class CppModuleObject : public Object {
public:
	CppModuleObject(Runtime* runtime);

	void AddExportMethod(Runtime* runtime, std::string_view name, CppFunction function);

};

} // namespace mjs
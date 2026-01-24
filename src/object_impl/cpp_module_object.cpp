#include <mjs/object_impl/cpp_module_object.h>

#include <mjs/runtime.h>

namespace mjs {

CppModuleObject::CppModuleObject(Runtime* runtime) :
	Object(runtime, ClassId::kCppModuleObject) {}

void CppModuleObject::AddExportMethod(Runtime* runtime, const char* name, CppFunction function) {
	auto const_index = runtime->global_const_pool().Insert(Value(name));
	SetProperty(runtime, const_index, Value(function));
}

} // namespace mjs
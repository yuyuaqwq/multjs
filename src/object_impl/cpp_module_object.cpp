#include <mjs/object_impl/cpp_module_object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

CppModuleObject::CppModuleObject(Context* context) :
	Object(context, ClassId::kCppModuleObject) {}

} // namespace mjs
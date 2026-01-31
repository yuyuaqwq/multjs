#include <mjs/class_def_impl/function_object_class_def.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

FunctionObjectClassDef::FunctionObjectClassDef(Runtime* runtime)
    : ClassDef(runtime, ClassId::kFunctionObject, "Function")
{
    // Function.prototype.__proto__ = Object.prototype
    prototype_.object().SetPrototype(&runtime->default_context(), runtime->class_def_table()[ClassId::kObject].prototype());

    runtime->class_def_table()[ClassId::kObject].constructor().object().SetPrototype(&runtime->default_context(), prototype_);
}

} // namespace mjs

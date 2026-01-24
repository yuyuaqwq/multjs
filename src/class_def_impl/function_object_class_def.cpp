#include <mjs/class_def_impl/function_object_class_def.h>

#include <mjs/context.h>

namespace mjs {

FunctionObjectClassDef::FunctionObjectClassDef(Runtime* runtime)
    : ClassDef(runtime, ClassId::kFunctionObject, "Function")
{
    // Function.prototype.__proto__ = Object.prototype
    prototype_.object().SetPrototype(runtime, runtime->class_def_table()[ClassId::kObject].prototype());

    runtime->class_def_table()[ClassId::kObject].constructor().object().SetPrototype(runtime, prototype_);
}

} // namespace mjs

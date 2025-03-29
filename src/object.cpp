#include <mjs/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

void Object::NewMethod(Value&& name, Value func) {
    SetProperty(std::move(name), Value(func));
}

} // namespace mjs
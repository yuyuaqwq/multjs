#pragma once

#include <mjs/opcode.h>
#include <mjs/generator_object.h>

namespace mjs {

class Context;
class AsyncObject : public GeneratorObject {
public:
    AsyncObject(Context* context, const Value& function)
        : GeneratorObject(context, function) {}

    virtual ClassId class_id() const override { return ClassId::kAsync; }

private:

};

} // namespace mjs
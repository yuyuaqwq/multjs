#pragma once

#include <mjs/opcode.h>
#include <mjs/generator_object.h>
#include <mjs/stack_frame.h>


namespace mjs {

class Context;
class Async : public GeneratorObject {
public:
    Async(Context* context, const Value& function)
        : GeneratorObject(context, function) {}

    virtual ClassId class_id() const override { return ClassId::kAsync; }

private:

};

} // namespace mjs
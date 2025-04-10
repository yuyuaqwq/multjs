#pragma once

#include <mjs/opcode.h>
#include <mjs/generator_object.h>
#include <mjs/promise_object.h>

namespace mjs {

class Context;
class AsyncObject : public GeneratorObject {
public:
    AsyncObject(Context* context, const Value& function)
        : GeneratorObject(context, function)
    {
        res_promise_ = Value(new PromiseObject(context, Value()));
    }

    virtual ClassId class_id() const override { return ClassId::kAsync; }

    const auto& res_promise() const { return res_promise_; }
    auto& res_promise() { return res_promise_; }

private:
    Value res_promise_;
};

} // namespace mjs
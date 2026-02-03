#pragma once

#include <functional>

#include <mjs/opcode.h>
#include <mjs/value/object/generator_object.h>
#include <mjs/value/object/promise_object.h>

namespace mjs {

class Context;
class AsyncObject : public GeneratorObject {
private:
    AsyncObject(Context* context, const Value& function);

public:
    void GCTraverse(Context* context, GCTraverseCallback callback) override;

    Value ToString(Context* context) override;

    const auto& res_promise() const { return res_promise_; }

    auto& res_promise() { return res_promise_; }

private:
    friend class GCManager;

    Value res_promise_;
};

} // namespace mjs
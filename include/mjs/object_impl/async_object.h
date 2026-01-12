#pragma once

#include <mjs/opcode.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/object_impl/promise_object.h>

namespace mjs {

class Context;
class AsyncObject : public GeneratorObject {
private:
    AsyncObject(Context* context, const Value& function)
        : GeneratorObject(context, function)
    {
        res_promise_ = Value(PromiseObject::New(context, Value()));
    }

public:
    void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
        GeneratorObject::GCForEachChild(context, list, callback);
        callback(context, list, res_promise_);
    }

    Value ToString(Context* context) override {
        return Value(String::Format("asnyc_object:{}", function_def().name()));
    }

    const auto& res_promise() const { return res_promise_; }
    auto& res_promise() { return res_promise_; }

    static AsyncObject* New(Context* context, const Value& function) {
        return new AsyncObject(context, function);
    }

private:
    Value res_promise_;
};

} // namespace mjs
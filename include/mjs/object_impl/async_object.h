#pragma once

#include <mjs/opcode.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/object_impl/promise_object.h>

namespace mjs {

class Context;
class AsyncObject : public GeneratorObject {
public:
    AsyncObject(Context* context, const Value& function)
        : GeneratorObject(context, function)
    {
        res_promise_ = Value(new PromiseObject(context, Value()));
    }

    void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
        GeneratorObject::GCForEachChild(context, list, callback);
        callback(context, list, res_promise_);
    }

    Value ToString(Context* context) override {
        return Value(String::format("asnyc_object:{}", function_def().name()));
    }

    ClassId class_id() const override { return ClassId::kAsyncObject; }

    const auto& res_promise() const { return res_promise_; }
    auto& res_promise() { return res_promise_; }

private:
    Value res_promise_;
};

} // namespace mjs
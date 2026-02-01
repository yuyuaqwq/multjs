#pragma once

#include <functional>

#include <mjs/opcode.h>
#include <mjs/value/object/generator_object.h>
#include <mjs/value/object/promise_object.h>

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
    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        // 先调用父类方法遍历属性
        GeneratorObject::GCTraverse(context, callback);

        // 遍历结果promise
        callback(context, res_promise_);
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
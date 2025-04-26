#pragma once

#include <mjs/object/object.h>

namespace mjs {

class ArrayObject : public Object {
public:
    ArrayObject(Runtime* runtime, size_t length)
        : Object(runtime)
        , values_(length) {}

    ArrayObject(Context* context, size_t length)
        : Object(context)
        , values_(length) {}

    void SetIndexed(Context* context, const Value& key, Value&& val) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        values_[key.i64()] = std::move(val);
    }

    Value* GetIndexed(Context* context, const Value& key) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        return &values_[key.i64()];
    }

    Object* New(Context* context) override {
        auto obj = new ArrayObject(context, values_.size());
        return obj;
    }


    Object* Copy(Object* new_obj, Context* context) override {
        auto* arr_obj = static_cast<ArrayObject*>(new_obj);
        arr_obj->values_ = values_;
        return Object::Copy(arr_obj, context);
    }

private:
    std::vector<Value> values_;
};

} // namespace mjs
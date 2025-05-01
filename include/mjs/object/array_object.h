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

    void SetComputedProperty(Context* context, const Value& key, Value&& val) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        values_[key.i64()] = std::move(val);
    }

    Value* GetComputedProperty(Context* context, const Value& key) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        return &values_[key.i64()];
    }

    void Push(Context* context, Value val) {
        values_.push_back(val);
    }

    Value Pop(Context* context) {
        auto back = std::move(values_.back());
        values_.pop_back();
        return back;
    }

    void ForEach(Context* context, Value callback) {
        
    }

    Value& operator[](size_t index) {
        return values_[index];
    }

    const Value& operator[](size_t index) const {
        return values_[index];
    }

    size_t size() const { return values_.size(); }

private:
    std::vector<Value> values_;
};

} // namespace mjs
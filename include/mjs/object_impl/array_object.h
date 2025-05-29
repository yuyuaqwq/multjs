#pragma once

#include <mjs/object.h>
#include <mjs/class_def_impl/array_object_class_def.h>

namespace mjs {

class ArrayObject : public Object {
private:
    ArrayObject(Context* context, size_t length)
        : Object(context, ClassId::kArrayObject)
        , values_(length) {}

public:
    bool GetProperty(Context* context, ConstIndex key, Value* value) override {
        if (key == GetClassDef<ArrayObjectClassDef>(&context->runtime()).length_const_index()) {
            *value = Value(length());
            return true;
        }
        return Object::GetProperty(context, key, value);
    }

    void SetComputedProperty(Context* context, const Value& key, Value&& value) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        values_[key.i64()] = std::move(value);
    }

    bool GetComputedProperty(Context* context, const Value& key, Value* value) override {
        if (key.i64() < 0 || key.i64() > values_.size()) {
            // throw;
        }
        *value = values_[key.i64()];
        return true;
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

    size_t length() const { return values_.size(); }

    Value& operator[](size_t index) {
        return values_[index];
    }

    const Value& operator[](size_t index) const {
        return values_[index];
    }

    virtual ClassId class_id() const { return ClassId::kArrayObject; }


    static ArrayObject* New(Context* context, std::initializer_list<Value> values) {
        auto arr_obj = new ArrayObject(context, values.size());
        size_t i = 0;
        for (auto& value : values) {
            arr_obj->operator[](i++) = value;
        }
        return arr_obj;
    }

    static ArrayObject* New(Context* context, size_t count) {
        return new ArrayObject(context, count);
    }

private:
    std::vector<Value> values_;
};

} // namespace mjs
#include <mjs/object_impl/array_object.h>

#include <mjs/context.h>
#include <mjs/error.h>

namespace mjs {

bool ArrayObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    if (key == GetClassDef<ArrayObjectClassDef>(&context->runtime()).length_const_index()) {
        *value = Value(length());
        return true;
    }
    return Object::GetProperty(context, key, value);
}

bool ArrayObject::GetComputedProperty(Context* context, const Value& key, Value* value) {
    if (!key.IsInt64() || key.i64() < 0 || key.i64() >= values_.size()) {
        *value = Error::Throw(context, "Not a valid index.");
        return false; // or throw an error
    }
    *value = values_[key.i64()];
    return true;
}

} // namespace mjs
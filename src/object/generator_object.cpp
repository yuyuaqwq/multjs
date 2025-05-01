#include <mjs/object/generator_object.h>

#include <mjs/context.h>

namespace mjs {

GeneratorObject::GeneratorObject(Context* context, const Value& function)
    : Object(context)
    , function_(function)
    , stack_(0) {}

void GeneratorObject::GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
    Object::GCForEachChild(context, list, callback);
    callback(context, list, function_);
    for (auto& val : stack_.vector()) {
        callback(context, list, val);
    }
}

Value GeneratorObject::MakeReturnObject(Context* context, Value&& ret_value) {
    // { value: $_, done: $boolean }
    //if (ret_obj_.IsUndefined()) {
    //    ret_obj_ = Value(new Object());
    //}

    // 每次都得new
    auto ret_obj = Value(new Object(context));

    auto& class_def = context->runtime().class_def_table().at(class_id()).get<GeneratorClassDef>();

    ret_obj.object().SetProperty(context, class_def.value_const_idx(), std::move(ret_value));
    ret_obj.object().SetProperty(context, class_def.done_const_idx(), Value(IsClosed()));
    return ret_obj;
}

void GeneratorObject::Next(Context* context) {
    auto func = Value(ValueType::kGeneratorNext);
    std::initializer_list<Value> args = {};
    context->CallFunction(&func, Value(this), args.begin(), args.end());
}

} // namespace mjs
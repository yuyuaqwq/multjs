#include <mjs/object_impl/generator_object.h>

#include <mjs/context.h>

namespace mjs {

GeneratorObject::GeneratorObject(Context* context, const Value& function)
    : Object(context, ClassId::kGeneratorObject)
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

    // 未来优化，思路：
    // 实现迭代器class
    // 返回Value类型是迭代器，而不会创建PropMap
    // 访问迭代器Value时，直接去查class的getprop
    // 这里的ret value，可能需要保存到GeneratorObject里
    // set的时候再提升为object

    auto ret_obj = Value(Object::New(context));

    auto& class_def = context->runtime().class_def_table()[ClassId::kGeneratorObject].get<GeneratorObjectClassDef>();

    ret_obj.object().SetProperty(context, class_def.value_const_index(), std::move(ret_value));
    ret_obj.object().SetProperty(context, class_def.done_const_index(), Value(IsClosed()));
    return ret_obj;
}

void GeneratorObject::Next(Context* context) {
    auto func = Value(ValueType::kGeneratorNext);
    std::initializer_list<Value> args = {};
    context->CallFunction(&func, Value(this), args.begin(), args.end());
}

} // namespace mjs
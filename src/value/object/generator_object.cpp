#include <mjs/value/object/generator_object.h>

#include <mjs/context.h>
#include <mjs/gc/handle.h>

namespace mjs {

GeneratorObject::GeneratorObject(Context* context, const Value& function)
    : Object(context, ClassId::kGeneratorObject)
    , function_(function)
    , stack_(0) {}

void GeneratorObject::GCTraverse(Context* context, GCTraverseCallback callback) {
    // 先调用父类方法遍历属性
    Object::GCTraverse(context, callback);

    // 遍历function_引用
    callback(context, &function_);

    // 遍历栈中的所有值
    for (auto& val : stack_.vector()) {
        callback(context, &val);
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

    GCHandleScope<1> scope(context);
    auto ret_obj = scope.New<Object>();

    ret_obj->SetProperty(context, ConstIndexEmbedded::kValue, std::move(ret_value));
    ret_obj->SetProperty(context, ConstIndexEmbedded::kDone, Value(IsClosed()));
    return scope.Close(ret_obj);
}

void GeneratorObject::Next(Context* context) {
    auto func = Value(ValueType::kGeneratorNext);
    std::initializer_list<Value> args = {};
    context->CallFunction(&func, Value(this), args.begin(), args.end());
}

} // namespace mjs
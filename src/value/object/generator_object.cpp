#include <mjs/value/object/generator_object.h>

#include <mjs/context.h>

namespace mjs {

GeneratorObject::GeneratorObject(Context* context, const Value& function, GCObjectType gc_type)
    : Object(context, ClassId::kGeneratorObject, gc_type)
    , function_(function)
    , stack_(0) {}

void GeneratorObject::GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
    Object::GCForEachChild(context, list, callback);
    callback(context, list, function_);
    for (auto& val : stack_.vector()) {
        callback(context, list, val);
    }
}

void GeneratorObject::GCTraverse(Context* context, std::function<void(Context* ctx, Value& value)> callback) {
    // 先调用父类方法遍历属性
    Object::GCTraverse(context, callback);

    // 遍历function_引用
    callback(context, function_);

    // 遍历栈中的所有值
    for (auto& val : stack_.vector()) {
        callback(context, val);
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

    ret_obj.object().SetProperty(context, ConstIndexEmbedded::kValue, std::move(ret_value));
    ret_obj.object().SetProperty(context, ConstIndexEmbedded::kDone, Value(IsClosed()));
    return ret_obj;
}

void GeneratorObject::Next(Context* context) {
    auto func = Value(ValueType::kGeneratorNext);
    std::initializer_list<Value> args = {};
    context->CallFunction(&func, Value(this), args.begin(), args.end());
}

GeneratorObject* GeneratorObject::New(Context* context, const Value& function) {
    // 使用 GCHeap 分配内存
    GCHeap* heap = context->gc_manager().heap();

    // 计算需要分配的总大小
    size_t total_size = sizeof(GeneratorObject);

    // 分配原始内存，不构造 GCObject
    void* mem = heap->AllocateRaw(GCObjectType::kOther, total_size);
    if (!mem) {
        return nullptr;
    }

    // 使用 placement new 在分配的内存中构造 GeneratorObject
    // 这会先构造 GCObject 基类，然后构造 GeneratorObject 派生类
    return new (mem) GeneratorObject(context, function);
}

} // namespace mjs
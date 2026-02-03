#include <mjs/value/object/async_object.h>

#include <mjs/context.h>
#include <mjs/gc/handle.h>
#include <mjs/value/object/promise_object.h>

namespace mjs {

AsyncObject::AsyncObject(Context* context, const Value& function)
    : GeneratorObject(context, function)
{
    GCHandleScope<1> scope(context);
    auto promise = scope.New<PromiseObject>(Value());
    res_promise_ = promise.ToValue();
}

Value AsyncObject::ToString(Context* context) {
    return Value(String::Format("asnyc_object:{}", function_def().name()));
}

void AsyncObject::GCTraverse(Context* context, GCTraverseCallback callback) {
    // 先调用父类方法遍历属性
    GeneratorObject::GCTraverse(context, callback);

    // 遍历结果promise
    callback(context, &res_promise_);
}

} // namespace mjs
#include <mjs/value/object/promise_object.h>

#include <mjs/context.h>

namespace mjs {

PromiseObject::PromiseObject(Context* context, Value executor, GCObjectType gc_type)
    : Object(context, ClassId::kPromiseObject, gc_type)
{
    if (executor.IsUndefined()) return;

    // 在构造函数中使用当前this是危险行为，需要注意
    // 现在使用 GC，不需要手动引用计数管理
    {
        auto argv = {
            Value(ValueType::kPromiseResolve, this),
            Value(ValueType::kPromiseReject, this)
        };
        // 传递两个参数，resolve和reject
        auto result = context->CallFunction(&executor, Value(), argv.begin(), argv.end());
        if (result.IsException()) {
            Reject(context, result);
        }
    }
}

void PromiseObject::Resolve(Context* context, Value result) {
    if (!IsPending()) {
        return;
    }

    if (!UnwrapPromise(context, &result)) {
        // 解决一个未完成的PromiseObject，等待其完成
        return;
    }

    state_ = State::kFulfilled;
    result_or_reason_ = result;

    auto& microtask_queue = context->microtask_queue();
    while (!on_fulfill_callbacks_.empty()) {
        auto& job = on_fulfill_callbacks_.front();
        job.AddArg(result_or_reason_);
        microtask_queue.emplace_back(std::move(job));
        on_fulfill_callbacks_.pop_front();
    }

    while (!on_reject_callbacks_.empty()) {
        on_reject_callbacks_.pop_front();
    }
}

void PromiseObject::Reject(Context* context, Value reason) {
    if (!IsPending()) {
        return;
    }

    if (!UnwrapPromise(context, &reason)) {
        // 拒绝一个未完成的PromiseObject，等待其完成
        return;
    }

    state_ = State::kRejected;
    result_or_reason_ = reason.SetException();

    auto& microtask_queue = context->microtask_queue();
    while (!on_reject_callbacks_.empty()) {
        auto& job = on_reject_callbacks_.front();
        job.AddArg(result_or_reason_);
        microtask_queue.emplace_back(std::move(job));
        on_reject_callbacks_.pop_front();
    }

    while (!on_fulfill_callbacks_.empty()) {
        on_fulfill_callbacks_.pop_front();
    }
}


Value PromiseObject::Then(Context* context, Value on_fulfilled, Value on_rejected) {
    auto* new_promise = new PromiseObject(context, Value());

    // 如果当前then的回调被执行，还需要执行new_promise中的Resolve/Reject
    auto fulfilled_handler = Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        auto& new_promise = stack.this_val().promise();
        // 首先拿到on_fulfilled执行的返回值
        assert(par_count == 2);
        Value on_fulfilled = stack.get(0);

        auto argv = { stack.get(1) };
        auto on_fulfilled_result = context->CallFunction(&on_fulfilled, Value(), argv.begin(), argv.end());

        // 然后传递给new_promise
        new_promise.Resolve(context, on_fulfilled_result);

        return Value();
    });

    auto rejected_handler = Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        auto& new_promise = stack.this_val().promise();
        // 首先拿到on_rejected执行的返回值
        assert(par_count == 2);
        Value on_rejected = stack.get(0);

        auto argv = { stack.get(1) };
        auto on_rejected_reason = context->CallFunction(&on_rejected, Value(), argv.begin(), argv.end());

        // 然后传递给new_promise
        new_promise.Reject(context, on_rejected_reason);

        return Value();
    });

    auto& microtask_queue = context->microtask_queue();

    if (on_fulfilled.IsUndefined()) {
        on_fulfilled = Value([](Context* ctx, uint32_t, const StackFrame& stack) {
            return stack.get(0);
        });
    }
    if (on_rejected.IsUndefined()) {
        on_rejected = Value([](Context* ctx, uint32_t, const StackFrame& stack) {
            return stack.get(0).SetException();
        });
    }

    if (IsPending()) {
        // 挂起状态：注册回调
        auto fulfill_job = Job(std::move(fulfilled_handler), Value(new_promise));
        fulfill_job.AddArg(on_fulfilled);
        on_fulfill_callbacks_.emplace_back(std::move(fulfill_job));
        
        auto reject_job = Job(std::move(rejected_handler), Value(new_promise));
        reject_job.AddArg(on_rejected);
        on_reject_callbacks_.emplace_back(std::move(reject_job));
    }
    else if (IsFulfilled()) {
        // 已完成状态：只添加fulfill任务
        auto fulfill_job = Job(std::move(fulfilled_handler), Value(new_promise));
        fulfill_job.AddArg(on_fulfilled);
        fulfill_job.AddArg(result_or_reason_);
        microtask_queue.emplace_back(std::move(fulfill_job));
    }
    else if (IsRejected()) {
        // 已拒绝状态：只添加reject任务
        auto reject_job = Job(std::move(rejected_handler), Value(new_promise));
        reject_job.AddArg(on_rejected);
        reject_job.AddArg(result_or_reason_);
        microtask_queue.emplace_back(std::move(reject_job));
    }

    return Value(new_promise);
}


bool PromiseObject::UnwrapPromise(Context* context, Value* result) {
    if (!result->IsPromiseObject()) {
        return true;
    }
    auto& inner_promise = result->promise();
    if (&inner_promise == this) {
        Reject(context, Value("Cycle detected").SetException());
        return false;
    }
    if (inner_promise.IsPending()) {
        // 当前promise的状态由inner_promise决定，保持等待
        inner_promise.Then(context,
            Value(ValueType::kPromiseResolve, this),
            Value(ValueType::kPromiseReject, this));
        return false;
    }
    // 同步展开已完成的 Promise
    *result = inner_promise.IsFulfilled() ? inner_promise.result() : inner_promise.reason();

    // 已完成的Promise，其结果必定不是Promise，因为已经经过解包了
    assert(!result->IsPromiseObject());

    return true;
}

void PromiseObject::GCTraverse(Context* context, GCTraverseCallback callback) {
    // 先调用父类方法遍历属性
    Object::GCTraverse(context, callback);

    // 遍历回调队列
    on_fulfill_callbacks_.GCTraverse(context, callback);
    on_reject_callbacks_.GCTraverse(context, callback);

    // 遍历结果或原因
    callback(context, result_or_reason_);
}

PromiseObject* PromiseObject::New(Context* context, Value executor) {
    // 使用 GCHeap 分配内存
    GCHeap* heap = context->gc_manager().heap();

    // 计算需要分配的总大小
    size_t total_size = sizeof(PromiseObject);

    // 分配原始内存，不构造 GCObject
    void* mem = heap->AllocateRaw(GCObjectType::kOther, total_size);
    if (!mem) {
        return nullptr;
    }

    // 使用 placement new 在分配的内存中构造 PromiseObject
    // 这会先构造 GCObject 基类，然后构造 PromiseObject 派生类
    return new (mem) PromiseObject(context, std::move(executor));
}

} // namespace mjs
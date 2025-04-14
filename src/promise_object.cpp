#include <mjs/promise_object.h>

#include <mjs/context.h>

namespace mjs {

PromiseObject::PromiseObject(Context* context, Value executor)
    : Object(context) {
    if (executor.IsUndefined()) return;

    // 在构造函数中使用当前this是危险行为，需要注意
    // 避免Value(kPromiseResolve) 和 Value(kPromiseReject) 的析构导致当前对象释放，先引用
    Reference();
    {
        auto argv = {
            Value(ValueType::kPromiseResolve, this),
            Value(ValueType::kPromiseReject, this)
        };
        // 传递两个参数，resolve和reject
        context->Call(executor, Value(), argv.begin(), argv.end());
    }
    // 避免解引用释放对象
    WeakDereference();
}

void PromiseObject::Resolve(Context* context, Value value) {
    if (!IsPending()) {
        return;
    }

    state_ = State::kFulfilled;
    result_ = value;

    auto& microtask_queue = context->microtask_queue();
    while (!on_fulfill_callbacks_.empty()) {
        auto& job = on_fulfill_callbacks_.front();
        job.AddArg(value);
        microtask_queue.emplace_back(std::move(job));
        on_fulfill_callbacks_.pop_front();
    }

    while (!on_reject_callbacks_.empty()) {
        on_reject_callbacks_.pop_front();
    }
}

void PromiseObject::Reject(Context* context, Value value) {
    if (!IsPending()) {
        return;
    }

    state_ = State::kRejected;
    result_ = value;

    auto& microtask_queue = context->microtask_queue();
    while (!on_reject_callbacks_.empty()) {
        auto& job = on_reject_callbacks_.front();
        job.AddArg(value);
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
        auto on_fulfilled_result = context->Call(on_fulfilled, Value(), argv.begin(), argv.end());

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
        auto on_rejected_result = context->Call(on_rejected, Value(), argv.begin(), argv.end());

        // 然后传递给new_promise
        new_promise.Reject(context, on_rejected_result);

        return Value();
    });

    auto& microtask_queue = context->microtask_queue();

    if (!on_fulfilled.IsUndefined()) {
        auto fulfill_job = Job(std::move(fulfilled_handler), Value(new_promise));
        fulfill_job.AddArg(on_fulfilled);

        if (IsPending()) {
            on_fulfill_callbacks_.emplace_back(std::move(fulfill_job));
        }
        else if (IsFulfilled()) {
            fulfill_job.AddArg(result_);
            microtask_queue.emplace_back(std::move(fulfill_job));
        }
    }
    if (!on_rejected.IsUndefined()) {
        auto reject_job = Job(std::move(rejected_handler), Value(new_promise));
        reject_job.AddArg(on_rejected);

        if (IsPending()) {
            on_reject_callbacks_.emplace_back(std::move(reject_job));
        }
        else if (IsRejected()) {
            reject_job.AddArg(result_);
            microtask_queue.emplace_back(std::move(reject_job));
        }
    }

    return Value(new_promise);
}

} // namespace mjs
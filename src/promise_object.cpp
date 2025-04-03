#include <mjs/promise_object.h>

#include <mjs/context.h>

namespace mjs {

PromiseObject::PromiseObject(Context* context, Value resolve_func, Value reject_func)
    : resolve_func_(std::move(resolve_func))
    , reject_func_(std::move(reject_func)) {}

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
        microtask_queue.emplace(std::move(job));
        on_fulfill_callbacks_.pop();
    }

    while (!on_reject_callbacks_.empty()) {
        on_reject_callbacks_.pop();
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
        microtask_queue.emplace(std::move(job));
        on_reject_callbacks_.pop();
    }

    while (!on_fulfill_callbacks_.empty()) {
        on_fulfill_callbacks_.pop();
    }
}

Value PromiseObject::Then(Context* context, Value on_fulfilled, Value on_rejected) {
    auto* new_promise = new PromiseObject(context, Value(), Value());

    // 如果当前then的回调被执行，还需要执行new_promise中的Resolve/Reject
    auto fulfilled_handler = Value([](Context* context, const Value& this_val, uint32_t par_count, const StackFrame& stack) -> Value {
        auto& new_promise = this_val.promise();
        // 首先拿到on_fulfilled执行的返回值
        assert(par_count == 2);
        Value on_fulfilled = stack.get(0);
        Value result = stack.get(1);

        auto on_fulfilled_result = context->Call(on_fulfilled, Value(), { result });

        // 然后传递给new_promise
        new_promise.Resolve(context, on_fulfilled_result);

        return Value();
    });

    auto rejected_handler = Value([](Context* context, const Value& this_val, uint32_t par_count, const StackFrame& stack) -> Value {
        auto& new_promise = this_val.promise();
        // 首先拿到on_rejected执行的返回值
        assert(par_count == 2);
        Value on_rejected = stack.get(0);
        Value result = stack.get(1);

        auto on_rejected_result = context->Call(on_rejected, Value(), { result });

        // 然后传递给new_promise
        new_promise.Reject(context, on_rejected_result);

        return Value();
    });

    auto& microtask_queue = context->microtask_queue();

    if (on_fulfilled.IsFunctionObject()) {
        auto fulfill_job = Job(std::move(fulfilled_handler), Value(new_promise));
        fulfill_job.AddArg(on_fulfilled);

        if (IsPending()) {
            on_fulfill_callbacks_.emplace(std::move(fulfill_job));
        }
        else if (IsFulfilled()) {
            fulfill_job.AddArg(result_);
            microtask_queue.emplace(std::move(fulfill_job));
        }
    }
    if (on_rejected.IsFunctionObject()) {
        auto reject_job = Job(std::move(rejected_handler), Value(new_promise));
        reject_job.AddArg(on_rejected);

        if (IsPending()) {
            on_reject_callbacks_.emplace(std::move(reject_job));
        }
        else if (IsRejected()) {
            reject_job.AddArg(result_);
            microtask_queue.emplace(std::move(reject_job));
        }
    }

    return Value(new_promise);
}

} // namespace mjs
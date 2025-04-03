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
    // todo
    auto* new_promise = new PromiseObject(context, Value(), Value());

    auto& microtask_queue = context->microtask_queue();
    if (IsFulfilled()) {
        auto job = Job(std::move(on_fulfilled), Value());
        job.AddArg(result_);
        microtask_queue.emplace(std::move(job));
        return Value(new_promise);
    }
    if (IsRejected()) {
        auto job = Job(std::move(on_rejected), Value());
        job.AddArg(result_);
        microtask_queue.emplace(std::move(job));
        return Value(new_promise);
    }

    if (on_fulfilled.IsFunctionObject()) {
        auto on_fulfill_job = Job(on_fulfilled, Value());
        if (IsPending()) {
            on_fulfill_callbacks_.emplace(std::move(on_fulfill_job));
        }
        else {
            microtask_queue.emplace(std::move(on_fulfill_job));
        }
    }
    if (on_rejected.IsFunctionObject()) {
        auto on_reject_job = Job(on_rejected, Value());
        if (IsPending()) {
            on_reject_callbacks_.emplace(std::move(on_reject_job));
        }
        else {
            microtask_queue.emplace(std::move(on_reject_job));
        }
    }

    return Value(new_promise);
}


} // namespace mjs
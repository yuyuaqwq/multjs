#include <mjs/promise_object.h>

#include <mjs/context.h>

namespace mjs {

PromiseObject::PromiseObject(Context* context, Value resolve_func, Value reject_func)
    : resolve_func_(std::move(resolve_func))
    , reject_func_(std::move(reject_func)) {}

void PromiseObject::Resolve(Context* context) {
    if (!IsPending()) {
        return;
    }
    state_ = State::kFulfilled;
    auto& microtask_queue = context->microtask_queue();
    while (!on_fulfill_callbacks_.empty()) {
        microtask_queue.emplace(std::move(on_fulfill_callbacks_.front()));
        on_fulfill_callbacks_.pop();
    }
}

void PromiseObject::Reject(Context* context) {
    if (!IsPending()) {
        return;
    }
    state_ = State::kRejected;
    auto& microtask_queue = context->microtask_queue();
    while (!on_reject_callbacks_.empty()) {
        microtask_queue.emplace(std::move(on_reject_callbacks_.front()));
        on_reject_callbacks_.pop();
    }
}

Value PromiseObject::Then(Context* context, Value on_fulfilled, Value on_rejected) {
    auto& microtask_queue = context->microtask_queue();
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
    return Value(this);
}


} // namespace mjs
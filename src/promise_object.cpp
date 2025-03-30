#include <mjs/promise_object.h>

#include <mjs/context.h>

namespace mjs {

PromiseObject::PromiseObject(Context* context, Value resolve_func, Value reject_func)
    : resolve_func_(std::move(resolve_func))
    , reject_func_(std::move(reject_func))
{
    NewMethod(Value("resolve"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
        auto& promise = this_val.promise();
        promise.Resolve(context);
        return Value();
    }));
    NewMethod(Value("reject"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
        auto& promise = this_val.promise();
        promise.Reject(context);
        return Value();
    }));
    NewMethod(Value("then"), Value([](Context* context, const Value& this_val, uint32_t par_count, StackFrame* stack) -> Value {
        auto& promise = this_val.promise();
        Value on_fulfilled;
        Value on_rejected;
        if (par_count > 0) {
            on_fulfilled = stack->pop();
        }
        if (par_count > 1) {
            on_rejected = stack->pop();
        }
        return promise.Then(context, on_fulfilled, on_rejected);
    }));
}


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
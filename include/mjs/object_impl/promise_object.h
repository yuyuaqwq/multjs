#pragma once

#include <mjs/job_queue.h>
#include <mjs/object.h>

#include <mjs/function_def.h>

namespace mjs {

class Context;
class PromiseObject : public Object {
public:
    PromiseObject(Context* context, Value executor);

    void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
        Object::GCForEachChild(context, list, callback);
        on_fulfill_callbacks_.ForEachChild(context, list, callback);
        on_reject_callbacks_.ForEachChild(context, list, callback);
        callback(context, list, result_);
    }

    void Resolve(Context* context, Value value);
    void Reject(Context* context, Value value);
    Value Then(Context* context, Value on_fulfilled, Value on_rejected);

    bool IsPending() {
        return state_ == State::kPending;
    }

    bool IsFulfilled() {
        return state_ == State::kFulfilled;
    }

    bool IsRejected() {
        return state_ == State::kRejected;
    }

    const auto& result() const { return result_; }
    void set_result(Value value) { result_ = std::move(value); }

    ClassId class_id() const override { return ClassId::kPromiseObject; }

private:
    enum class State {
        kPending = 0,
        kFulfilled = 1,
        kRejected = 2,
    } state_ = State::kPending;

    // pending job queue
    JobQueue on_fulfill_callbacks_;
    JobQueue on_reject_callbacks_;

    Value result_;
};

} // namespace mjs
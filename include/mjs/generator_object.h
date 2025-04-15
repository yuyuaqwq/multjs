#pragma once

#include <mjs/opcode.h>
#include <mjs/object.h>
#include <mjs/stack_frame.h>

namespace mjs {

class Context;
class GeneratorObject : public Object {
public:
    GeneratorObject(Context* context, const Value& function)
        : Object(context)
        , function_(function)
        , stack_(0) {}

    virtual ~GeneratorObject() override = default;

    virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) override {
        Object::ForEachChild(list, callback);
        callback(list, function_);
        for (auto& val : stack_.vector()) {
            callback(list, val);
        }
    }

    bool IsSuspended() const { return state_ == State::kSuspended; }
    bool IsExecuting() const { return state_ == State::kExecuting; }
    bool IsClosed() const { return state_ == State::kClosed; }
    
    void SetExecuting() {
        assert(state_ == State::kSuspended || state_ == State::kExecuting);
        state_ = State::kExecuting;
    }
    void SetClosed() {
        assert(state_ == State::kSuspended || state_ == State::kExecuting);
        state_ = State::kClosed;
    }

    Value MakeReturnObject(Context* context, Value&& ret_value) {
        // { value: $_, done: $boolean }
        //if (ret_obj_.IsUndefined()) {
        //    ret_obj_ = Value(new Object());
        //}

        // 每次都得new
        auto ret_obj = Value(new Object(context));
        ret_obj.object().SetProperty(context, Value("value"), std::move(ret_value));
        ret_obj.object().SetProperty(context, Value("done"), Value(IsClosed()));
        return ret_obj;
    }

    void Next(Context* context) {
        // 执行Next

    }

    virtual ClassId class_id() const override { return ClassId::kGenerator; }

    auto& stack() { return stack_; }

    auto& function_def() { 
        if (function_.IsFunctionDef()) {
            return function_.function_def();
        }
        return function_.function().function_def();
    }

    auto function() { return function_; }

    auto pc() const { return pc_; }
    void set_pc(Pc pc) { pc_ = pc; }

private:
    Value function_;        // 生成器函数定义/函数对象
    Pc pc_ = 0;         // 当前pc
    Stack stack_;       // 保存的栈

    // Value ret_obj_;

    enum class State {
        kSuspended,
        kExecuting,
        kClosed,
    } state_ = State::kSuspended;
};

} // namespace mjs
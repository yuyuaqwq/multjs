#pragma once

#include <mjs/runtime.h>
#include <mjs/opcode.h>
#include <mjs/object.h>
#include <mjs/stack_frame.h>


namespace mjs {

class GeneratorObject : public Object {
public:
    GeneratorObject(const Runtime& runtime, const Value& func)
        : func_(func), stack_(0)
    {
        NewMethod(Value("next"), Value(ValueType::kGeneratorNext, this));
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

    Value MakeReturnObject(Value&& ret_value) {
        // { value: $_, done: $boolean }
        //if (ret_obj_.IsUndefined()) {
        //    ret_obj_ = Value(new Object());
        //}

        // 每次都得new
        auto ret_obj = Value(new Object());
        ret_obj.object().SetProperty(Value("value"), std::move(ret_value));
        ret_obj.object().SetProperty(Value("done"), Value(IsClosed()));
        return ret_obj;
    }

    auto& stack() { return stack_; }

    auto function_def() { 
        if (func_.IsFunctionDef()) {
            return func_.function_def();
        }
        return func_.function()->func_def_;
    }

    auto function() { return func_; }

    auto pc() const { return pc_; }
    void set_pc(Pc pc) { pc_ = pc; }

private:
    Value func_;        // 生成器函数定义/函数对象
    Pc pc_ = 0;         // 当前pc
    Stack stack_;       // 栈

    // Value ret_obj_;

    enum class State {
        kSuspended,
        kExecuting,
        kClosed,
    } state_ = State::kSuspended;
};



} // namespace mjs
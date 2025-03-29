#pragma once

#include <mjs/runtime.h>
#include <mjs/object.h>
#include <mjs/stack_frame.h>
#include <mjs/opcode_type.h>

namespace mjs {

class GeneratorObject : public Object {
public:
    GeneratorObject(const Runtime& runtime, const Value& func)
        : func_(func)
        , stack_(kStackDefaultSize)
    {
        NewMethod(Value("next"), Value(ValueType::kGeneratorNext));
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

    bool IsClosed() const { return state_ == State::kClosed; }

    void SetExecuting() { 
        assert(state_ == State::kSuspended || state_ == State::kExecuting);
        state_ = State::kExecuting;
    }
    void SetClosed() { 
        assert(state_ == State::kSuspended || state_ == State::kExecuting);
        state_ = State::kClosed; 
    }

private:
    static constexpr size_t kStackDefaultSize = 16;
    Value func_;        // 生成器函数定义/函数对象
    Pc pc_ = 0;         // 当前pc
    Stack stack_;       // 栈

    enum class State {
        kSuspended,
        kExecuting,
        kClosed,
    } state_ = State::kSuspended;
};



} // namespace mjs
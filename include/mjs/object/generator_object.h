#pragma once

#include <mjs/opcode.h>
#include <mjs/stack_frame.h>
#include <mjs/object/object.h>
#include <mjs/object/function_object.h>
#include <mjs/class_def/generator_class_def.h>

namespace mjs {

class Context;
class GeneratorObject : public Object {
public:
    GeneratorObject(Context* context, const Value& function);

    ~GeneratorObject() override = default;

    void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override;

    Value ToString() override {
        return Value(String::format("generator_object:{}", function_def().name()));
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

    Value MakeReturnObject(Context* context, Value&& ret_value);

    void Next(Context* context);


    auto& stack() { return stack_; }

    FunctionDef& function_def() { 
        if (function_.IsFunctionDef()) {
            return function_.function_def();
        }
        return function_.function().function_def();
    }

    auto function() { return function_; }

    auto pc() const { return pc_; }
    void set_pc(Pc pc) { pc_ = pc; }

    ClassId class_id() const override { return ClassId::kGenerator; }

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
#pragma once

#include <mjs/const_def.h>

namespace mjs {

// symbol存储一个context，用于Value析构时，递减const_index的引用计数
class Context;
class Symbol {
public:
    Symbol() = default;
    
    Symbol(Context* context) : context_(context) {}

    Context& context() { return *context_; }

private:
    Context* context_ = nullptr;
};

} // namespace mjs
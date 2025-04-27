#pragma once

#include <mjs/const_def.h>

namespace mjs {

// symbol�洢һ��context������Value����ʱ���ݼ�const_index�����ü���
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
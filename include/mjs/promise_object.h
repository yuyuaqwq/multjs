#pragma once

#include <mjs/object.h>

namespace mjs {

class PromiseObject : public Object {
public:
    enum class State {
        kPending = 0,
        kFulfilled = 1,
        kRejected = 2,
    };


private:
    // resolve��reject����
    Value resolving_funcs_[2];
    // fulfill��reject��Ӧ
    Value reactions_[2];
    State state_;
};

} // namespace mjs
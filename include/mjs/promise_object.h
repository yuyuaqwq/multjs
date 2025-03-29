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
    // resolve和reject函数
    Value resolving_funcs_[2];
    // fulfill和reject反应
    Value reactions_[2];
    State state_;
};

} // namespace mjs
#pragma once

#include <mjs/const_def.h>
#include <mjs/reference_counter.h>

namespace mjs {

class Symbol : public ReferenceCounter {
public:
    Symbol(ConstIndex str_idx)
        : str_idx_(str_idx) {}

    ConstIndex str_idx() const { return str_idx_; }

private:
    ConstIndex str_idx_;
};

} // namespace mjs
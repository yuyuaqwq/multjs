#pragma once

#include <mjs/reference_counter.h>

namespace mjs {

class Context;
class Symbol : public ReferenceCounter<Symbol> {};

} // namespace mjs
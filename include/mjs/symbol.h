#pragma once

#include <mjs/string.h>

namespace mjs {

class Context;
class Symbol : public ReferenceCounter<Symbol> {};

} // namespace mjs
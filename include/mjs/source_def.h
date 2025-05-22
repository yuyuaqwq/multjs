#pragma once

#include <cstdint>

namespace mjs {

using SourcePos = uint32_t;
using SourceLine = uint32_t;

constexpr SourcePos kInvalidSourcePos = 0xffffffff;
constexpr SourceLine kInvalidSourceLine = 0xffffffff;

} // namespace mjs
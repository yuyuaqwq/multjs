#include <mjs/shape_property.h>

namespace mjs {

ShapeProperty::ShapeProperty(uint32_t flags, ConstIndex const_index)
    : flags_(flags)
    , const_index_(const_index) {}

} // namespace mjs
#include <mjs/shape.h>
#include <mjs/context.h>

#include <cassert>

namespace mjs {

Shape::Shape(ShapeManager* shape_manager)
    : ReferenceCounter()
    , shape_manager_(shape_manager)
    , property_size_(0)
    , property_map_(nullptr)
    , parent_shape_(nullptr) {}

Shape::Shape(Shape* parent_shape, uint32_t property_size)
    : ReferenceCounter()
    , parent_shape_(parent_shape)
    , shape_manager_(parent_shape->shape_manager_)
    , property_size_(property_size)
    , property_map_(parent_shape->property_map_)
{
    if (!property_map_) {
        property_map_ = new ShapePropertyHashTable;
    }
}

Shape::~Shape() {
    if (parent_shape_) {
        // 从父节点的过渡表中移除
        // base_shape->parent_shape()->transition_table().erase(base_shape->parent_transition_table_iter());
        // 因为只有add才会导致创建新的shape，上次add的一定在末尾
        auto success = parent_shape_->transtion_table_.Delete(GetProperty(property_size_ - 1).const_index());
        assert(success);

        // 释放property_map_
        if (property_map_ != parent_shape_->property_map_) {
            assert(property_map_);
            property_map_->DereferenceConstValue(&shape_manager_->context());
            delete property_map_;
        }

        parent_shape_->Dereference();
    }
    else {
        assert(!property_map_);
    }
}

const int Shape::Find(ConstIndex const_index) const {
    if (!property_map_) { return -1; }
    return property_map_->Find(const_index, property_size_);
}

void Shape::Add(ShapeProperty&& prop) {
    shape_manager_->context().ReferenceConstValue(prop.const_index());
    return property_map_->Add(std::move(prop));
}

const ShapeProperty& Shape::GetProperty(int32_t idx) const {
    return property_map_->GetProperty(idx);
}

} // namespace mjs
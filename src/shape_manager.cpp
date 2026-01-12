#include <mjs/shape_manager.h>

#include <mjs/context.h>
#include <mjs/class_def_impl/object_class_def.h>

namespace mjs {

ShapeManager::ShapeManager(Context* context)
    : context_(context)
{
    empty_shape_ = new Shape(this);
    empty_shape_->Reference();
}

ShapeManager::~ShapeManager() {
    empty_shape_->Dereference();
}

PropertySlotIndex ShapeManager::AddProperty(Shape** base_shape_ptr, ShapeProperty&& property) {
    auto base_shape = *base_shape_ptr;
    if (base_shape != empty_shape_) {
        auto index = base_shape->Find(property.const_index());
        if (index != kPropertySlotIndexInvalid) {
            // 已经存在的属性，返回
            return index;
        }
    }

    // 查找过渡表
    auto transition_shape = base_shape->transtion_table().Find(property.const_index());
    if (transition_shape) {
        base_shape->Dereference();
        *base_shape_ptr = transition_shape;
        (*base_shape_ptr)->Reference();
        return AddProperty(base_shape_ptr, std::move(property));
    }

    // 创建新的shape
    Shape* new_shape = new Shape(base_shape, base_shape->property_size() + 1);

    if (base_shape->transtion_table().Has()) {
        // 如果过渡表不为空，需要创建一个分支
        if (new_shape->property_map() == base_shape->property_map()) {
            new_shape->set_property_map(new ShapePropertyHashTable);
        }
        for (int32_t i = 0; i < base_shape->property_size(); ++i) {
            ShapeProperty property = base_shape->GetProperty(i);
            new_shape->Add(std::move(property));
        }
    }

    // 放到过渡表
    base_shape->transtion_table().Add(property.const_index(), new_shape);

    // 子节点引用父节点
    // base_shape->Dereference();
    *base_shape_ptr = new_shape;
    (*base_shape_ptr)->Reference();

    new_shape->Add(std::move(property));
    return new_shape->property_size() - 1;
}

} // namespace mjs
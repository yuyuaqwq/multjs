/**
 * @file shape.h
 * @brief JavaScript 形状系统定义 - 聚合头文件
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的形状类。
 */

#pragma once

#include <mjs/class_def.h>
#include <mjs/shape_property_hash_table.h>
#include <mjs/transition_table.h>

namespace mjs {

/**
 * @class Shape
 * @brief 形状类
 *
 * 表示对象的形状信息，包括父形状、原型、属性大小和过渡表等。
 * 继承自 ReferenceCounter，支持引用计数管理。
 *
 * @see ReferenceCounter 引用计数基类
 */
class ShapeManager;
class Shape : public ReferenceCounter<Shape> {
public:
    explicit Shape(ShapeManager* shape_manager);

    Shape(Shape* parent_shape, uint32_t property_size);

    ~Shape();

    const PropertySlotIndex Find(ConstIndex const_index) const;

    void Add(ShapeProperty&& prop);

    const ShapeProperty& GetProperty(PropertySlotIndex idx) const;

    ShapeManager* shape_manager() { return shape_manager_; }

    const ShapeManager* shape_manager() const { return shape_manager_; }

    Shape* parent_shape() const { return parent_shape_; }

    void set_parent_shape(Shape* parent_shape) { parent_shape_ = parent_shape; }

    ShapePropertyHashTable* property_map() const { return property_map_; }

    void set_property_map(ShapePropertyHashTable* property_map) { property_map_ = property_map; }

    auto& transtion_table() { return transtion_table_; }

    uint32_t property_size() const { return property_size_; }

private:
    ShapeManager* shape_manager_;
    Shape* parent_shape_;

    uint32_t property_size_;
    ShapePropertyHashTable* property_map_;

    TransitionTable transtion_table_;
};

} // namespace mjs
/**
 * @file shape_property_hash_table.h
 * @brief JavaScript 形状属性哈希表定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的形状属性哈希表类，用于高效存储和查找形状属性。
 */

#pragma once

#include <mjs/shape_property.h>

namespace mjs {

class Context;

using ShapeSlotIndex = int32_t;
constexpr ShapeSlotIndex kShapeSlotIndexInvalid = -1;


/**
 * @class ShapePropertyHashTable
 * @brief 形状属性哈希表类
 *
 * shape 链下的所有 shape 都指向一个哈希表，每个 Shape 控制 size 来限制查找范围。
 * 提供属性查找、添加和常量值引用计数管理功能。
 *
 * @note shape 链下的所有 shape 都指向一个哈希表
 * @note 每个 Shape 控制 size 来限制查找范围
 */
class ShapePropertyHashTable {
public:
	~ShapePropertyHashTable();

	const ShapeSlotIndex Find(ConstIndex const_index, uint32_t property_size) const;
	void Add(ShapeProperty&& prop);
	const ShapeProperty& GetProperty(ShapeSlotIndex idx);
	void DereferenceConstValue(Context* context);

private:
	uint32_t GetPower2(uint32_t n);
	double CalcLoadingFactor() const;
	void Rehash(uint32_t new_capacity);

private:
	static constexpr uint32_t kPropertiesMaxSize = 4;
	static constexpr double kLoadingFactor = 0.75f;

	uint32_t property_size_ = 0;
	uint32_t property_capacity_ = 0;
	ShapeProperty* properties_ = nullptr;

	uint32_t hash_mask_ = 0;
	uint32_t hash_capacity_ = 0;
	ShapeSlotIndex* slot_indices_ = nullptr;
};

} // namespace mjs
/**
 * @file shape.h
 * @brief JavaScript 形状系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的形状系统，包括形状属性、形状属性哈希表、
 * 过渡表、形状和形状管理器等组件，用于实现对象属性的高效存储和查找。
 */

#pragma once

#include <mjs/unordered_dense.h>

#include <mjs/reference_counter.h>

#include <mjs/value.h>
#include <mjs/class_def.h>

namespace mjs {

/**
 * @class ShapeProperty
 * @brief 形状属性类
 *
 * 表示对象的属性信息，包括属性标志和常量索引。
 * 用于形状系统中的属性管理。
 */
class ShapeProperty {
public:
	ShapeProperty() = default;
	ShapeProperty(uint32_t flags, ConstIndex const_index);
	~ShapeProperty() = default;

	uint32_t flags() const { return flags_; }
	void set_flags(uint32_t flags) { flags_ = flags; }

	ConstIndex const_index() const { return const_index_; }
	void set_const_index(ConstIndex const_index) { const_index_ = const_index; }

private:
	uint32_t flags_ = 0;
	ConstIndex const_index_;
};

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

	const int Find(ConstIndex const_index, uint32_t property_size) const;
	void Add(ShapeProperty&& prop);
	const ShapeProperty& GetProperty(int32_t idx);
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
	int32_t* slot_indices_ = nullptr;
};

/**
 * @class TransitionTable
 * @brief 过渡表类
 *
 * 管理形状之间的过渡关系，支持形状查找、添加和删除操作。
 * 过渡表不引用 ConstIndex，因为有过渡表就说明有属性到子 Shape，
 * 子 Shape 的 PropertyMap 中引用一份就行。
 *
 * @note 过渡表不引用 ConstIndex
 * @note 因为有过渡表就说明有属性到子 Shape，子 Shape 的 PropertyMap 中引用一份就行
 * @note Shape 释放的时候也会从父 Shape 的过渡表中删除
 */
class Shape;
class TransitionTable {
public:
	~TransitionTable() {
		assert(!Has());
		if (type_ == Type::kMap) {
			delete map_;
		}
	}

	bool Has() const;
	Shape* Find(ConstIndex key) const;
	void Add(ConstIndex key, Shape*);
	bool Delete(ConstIndex key);

private:
	enum class Type {
		kNone,
		kOne,
		kMap
	} type_ = Type::kNone;
	ConstIndex key_;
	union {
		Shape* shape_;
		ankerl::unordered_dense::map<ConstIndex, Shape*>* map_;
	};
};

/**
 * @class Shape
 * @brief 形状类
 *
 * 表示对象的形状信息，包括父形状、原型、类ID、属性大小和过渡表等。
 * 继承自 ReferenceCounter，支持引用计数管理。
 *
 * @see ReferenceCounter 引用计数基类
 */
class ShapeManager;
class Shape : public ReferenceCounter<Shape> {
public:
	Shape(ShapeManager* shape_manager);
	Shape(Shape* parent_shape, uint32_t property_size);
	~Shape();

	const int Find(ConstIndex const_index) const;
	void Add(ShapeProperty&& prop);
	const ShapeProperty& GetProperty(int32_t idx) const;

	Shape* parent_shape() const { return parent_shape_; }
	void set_parent_shape(Shape* parent_shape) { parent_shape_ = parent_shape; }

	ShapePropertyHashTable* property_map() const { return property_map_; }
	void set_property_map(ShapePropertyHashTable* property_map) { property_map_ = property_map; }

	auto& transtion_table() { return transtion_table_; }

	uint32_t property_size() const { return property_size_; }

	auto& shape_manager() { return shape_manager_; }

private:
	ShapeManager* shape_manager_;
	Shape* parent_shape_;

	Value prototype_;

	ClassId class_id_;

	uint32_t property_size_;
	ShapePropertyHashTable* property_map_;

	TransitionTable transtion_table_;
};


/**
 * @class ShapeManager
 * @brief 形状管理器类
 *
 * 管理形状的创建和属性添加，提供形状系统的统一管理接口。
 * 继承自 noncopyable，确保不可拷贝。
 *
 * @note 后续的修改方向是，引用计数归0不会回收对象
 * @note 只有 GC 的时候，递归到没有子节点的节点，且该节点引用计数为0，才能清除当前节点
 * @see noncopyable 不可拷贝基类
 */
class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);
	~ShapeManager();

	int AddProperty(Shape** base_shape, ShapeProperty&& property);

	auto& context() { return *context_; }
	Shape& empty_shape() { return *empty_shape_; }
	
private:
	Context* context_;
	Shape* empty_shape_;
};

} // namespace mjs
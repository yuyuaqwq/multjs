#pragma once

#include <mjs/unordered_dense.h>

#include <mjs/reference_counter.h>

#include <mjs/value.h>
#include <mjs/class_def.h>

namespace mjs {

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

// shape链下的所有shape，都指向一个哈希表
// 每个Shape控制size来限制查找范围

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

// 过渡表不引用ConstIndex
// 因为有过渡表就说明有属性到子Shape，子Shape的PropertyMap中引用一份就行
// Shape释放的时候也会从父Shape的过渡表中删除
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
		struct {
			Shape* shape_;
		};
		ankerl::unordered_dense::map<ConstIndex, Shape*>* map_;
	};
};

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


// 后续的修改方向是，引用计数归0不会回收对象
// 只有GC的时候，递归到没有子节点的节点，且该节点引用计数为0，才能清除当前节点
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
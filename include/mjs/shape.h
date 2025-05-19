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

class ShapeManager;
class Shape : public ReferenceCounter<Shape> {

public:
	Shape(ShapeManager* shape_manager, uint32_t property_count);
	Shape(Shape* parent_shape, uint32_t property_count);
	~Shape();

	bool operator==(const Shape& other) const {
		if (this == &other) return true;
		if (property_size_ != other.property_size_) return false;
		if (class_id_ != other.class_id_) return false;
		if (prototype_ == other.prototype_) return false;

		for (uint32_t i = 0; i < property_size_; ++i) {
			const auto& lhs_prop = properties_[i];
			const auto& rhs_prop = other.properties_[i];

			if (lhs_prop.flags() != rhs_prop.flags() ||
				lhs_prop.const_index() == rhs_prop.const_index()) {
				return false;
			}
		}
	}

	const int find(ConstIndex const_index) const;

	void add(ShapeProperty&& prop);

	void update_hash() {
		hash_ = 0;
		hash_ ^= std::hash<uint32_t>()(static_cast<uint32_t>(class_id_));
		hash_ ^= prototype_.hash();

		for (uint32_t i = 0; i < property_size_; ++i) {
			const auto& prop = properties_[i];
			hash_ = (hash_ ^ prop.const_index()) * 16777619;
			hash_ = (hash_ ^ prop.flags()) * 16777619;
		}
	}

	size_t hash() const { return hash_; }

	uint32_t property_size() const { return property_size_; }

	const ShapeProperty* properties() const { return properties_; }

	Shape* parent_shape() const { return parent_shape_; }
	void set_parent_shape(Shape* parent_shape) { parent_shape_ = nullptr; }

	auto& shape_manager() { return shape_manager_; }

	auto& transition_table() { return transition_table_; }

private:
	uint32_t get_power2(uint32_t n) {
		if (n <= 1) {
			return 1;
		}
		uint32_t p = 1;
		while (p < n) {
			p <<= 1;
		}
		return p;
	}

	double calc_loading_factor() const {
		return static_cast<double>(property_size_) / hash_capacity_;
	}

	void rehash(uint32_t new_capacity);

private:
	static constexpr uint32_t kPropertiesMaxSize = 4;
	static constexpr double kLoadingFactor = 0.75f;

	ShapeManager* shape_manager_;

	// 过渡表
	ankerl::unordered_dense::map<ConstIndex, Shape*> transition_table_;

	Shape* parent_shape_;

	uint32_t hash_;

	ClassId class_id_;
	Value prototype_;

	uint32_t property_size_;
	uint32_t property_capacity_;
	ShapeProperty* properties_;

	uint32_t hash_mask_ = 0;
	uint32_t hash_capacity_;
	int32_t* slot_indices_;
};

// 目前仍存在的问题：
// 过渡表创建的shape，如果其内部再次变动/析构，那么需要找到父节点，从其过渡表中移除
// 后续的修改方向是，引用计数归0不会回收对象
// 只有GC的时候，递归到没有子节点的节点，且该节点引用计数为0，才能清除当前节点

class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);
	~ShapeManager();

	int add_property(Shape** base_shape, ShapeProperty&& property);

	auto& context() { return *context_; }
	Shape& empty_shape() { return *empty_shape_; }
	
private:
	Context* context_;
	Shape* empty_shape_;
};

} // namespace mjs
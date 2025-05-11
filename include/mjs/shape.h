#pragma once

#include <mjs/unordered_dense.h>

#include <mjs/reference_counter.h>

#include <mjs/value.h>

namespace mjs {

class ShapeProperty {
public:
	ShapeProperty() = default;
	ShapeProperty(uint32_t flags, ConstIndex const_index, uint32_t slot_index)
		: flags_(flags)
		, const_index_(const_index) {}

	uint32_t flags() const { return flags_; }
	void set_flags(uint32_t flags) { flags_ = flags; }

	ConstIndex const_index() const { return const_index_; }
	void set_const_index(ConstIndex const_index) { const_index_ = const_index; }

private:
	uint32_t flags_;
	ConstIndex const_index_;
};

// 未来将prototype指针和class_id保存在这里
class Context;
class Shape : public ReferenceCounter {
public:
	Shape(Context* context, uint32_t property_count);
	~Shape();

	// 比较运算符
	bool operator==(const Shape& other) const {
		
	}

	// 获取属性数量
	uint32_t prop_size() const { return prop_size_; }

	// 获取属性数组
	const ShapeProperty* prop_array() const { return prop_array_; }

	// 获取哈希值
	size_t hash() const { return hash_; }

	// 计算哈希值
	void update_hash() {
		hash_ = 0;
		// 使用 const_index 和 flags 计算哈希值
		for (uint32_t i = 0; i < prop_size_; ++i) {
			const auto& prop = prop_array_[i];
			// 使用 FNV-1a 哈希算法
			hash_ = (hash_ ^ prop.const_index().value()) * 16777619;
			hash_ = (hash_ ^ prop.flags()) * 16777619;
		}
	}

	// 查找属性
	const ShapeProperty* find(ConstIndex const_index) const;

	// 添加属性
	void add(ShapeProperty&& prop);

	// 增量比较添加新属性后的Shape
	bool equal_new_shape(Shape* base_shape, ShapeProperty* property) {
		auto new_shape_prop_size = base_shape->prop_size_ + 1;

		if (this == base_shape && property == nullptr) return false;
		if (prop_size_ != new_shape_prop_size) return false;
		if (class_id_ != base_shape->class_id_) return false;
		if (prototype_ == base_shape->prototype_) return false;

		// 比较所有属性
		for (uint32_t i = 0; i < prop_capacity_; ++i) {
			const auto& lhs_prop = prop_array_[i];
			const auto& rhs_prop = base_shape->prop_array_[i];

			if (lhs_prop.const_index().is_invalid() && rhs_prop.const_index().is_invalid()) {
				continue;
			}

			if (lhs_prop.flags() != rhs_prop.flags() ||
				lhs_prop.const_index() != rhs_prop.const_index()) {
				return false;
			}
		}

		return true;
	}

	// 增量计算添加新属性后的哈希值
	static size_t calc_new_shape_hash(Shape* base_shape, const ShapeProperty& property) {
		size_t hash = base_shape->hash();
		hash = (hash ^ property.const_index().value()) * 16777619;
		hash = (hash ^ property.flags()) * 16777619;
		return hash;
	}

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

	// 检查是否需要扩容
	bool need_rehash() const {
		return prop_size_ > prop_capacity_ * kLoadFactor;
	}

private:
	uint32_t hash_;

	ClassId class_id_;
	Value prototype_;

	uint32_t prop_size_;
	ShapeProperty* prop_array_;

	ankerl::unordered_dense::set<uint32_t> prop_hash_;

	static constexpr uint32_t kPropArrayMaxSize = 4;
};


class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);
	~ShapeManager();

	// 创建新的Shape
	Shape* new_shape();

	// 查找或创建带有新属性的Shape
	Shape* find_or_create_shape(Shape* base_shape, const ShapeProperty& property);

private:
	Context* context_;
	ankerl::unordered_dense::set<Shape> shape_cache_;
};

} // namespace mjs
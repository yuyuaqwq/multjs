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
		, const_index_(const_index)
		, slot_index_(slot_index) {}

	uint32_t flags() const { return flags_; }
	void set_flags(uint32_t flags) { flags_ = flags; }

	ConstIndex const_index() const { return const_index_; }
	void set_const_index(ConstIndex const_index) { const_index_ = const_index; }

	uint32_t slot_index() const { return slot_index_; }
	void set_slot_index(uint32_t slot_index) { slot_index_ = slot_index; }

private:
	uint32_t flags_;
	ConstIndex const_index_;
	uint32_t slot_index_;
};

class Context;
class Shape : public ReferenceCounter {
public:
	Shape(Context* context);
	~Shape();

	// 获取属性数量
	uint32_t prop_size() const { return prop_size_; }

	// 获取属性数组
	const ShapeProperty* prop_array() const { return prop_array_; }

	// 获取哈希值
	size_t hash() const { return hash_; }

	// 查找属性
	const ShapeProperty* find(ConstIndex const_index) const;

	// 添加属性
	void add(ShapeProperty&& prop);

	void rehash();

	// 删除属性
	void remove_property(ConstIndex const_index);


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

private:
	uint32_t hash_;
	uint32_t prop_hash_mask_ = 0;
	uint32_t prop_size_;
	uint32_t prop_capacity_;

	ShapeProperty* prop_array_;

	static constexpr uint32_t kPropArrayMaxSize = 4;
};

class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);
	~ShapeManager();

	// 创建新的Shape
	Shape* new_shape();

	// 查找或创建带有新属性的Shape
	Shape* find_or_create_shape(Shape* base_shape, ConstIndex const_index, uint32_t flags);

private:
	Context* context_;
	ankerl::unordered_dense::map<uint32_t, Shape*> shape_cache_;
};

} // namespace mjs
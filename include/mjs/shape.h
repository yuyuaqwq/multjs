#pragma once

#include <mjs/unordered_dense.h>

#include <mjs/reference_counter.h>

#include <mjs/value.h>
#include <mjs/class_def.h>

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


	bool const_index_equal(Context* context, const ConstIndex& rhs) const;

	static uint64_t const_index_hash(Context* context, const ConstIndex& const_index);

private:
	static const Value& GetPoolValue(Context* context, const ConstIndex& key);

private:
	uint32_t flags_;
	ConstIndex const_index_;
};

class Context;
class Shape : public ReferenceCounter {
public:
	Shape(ShapeManager* shape_manager, uint32_t property_count);
	~Shape();

	bool operator==(const Shape& other) const {
		
	}

	const ShapeProperty* find(ConstIndex const_index) const;

	void add(ShapeProperty&& prop);

	void update_hash() {
		hash_ = 0;
		hash_ ^= std::hash<uint32_t>()(property_size_);
		hash_ ^= std::hash<uint32_t>()(static_cast<uint32_t>(class_id_));
		hash_ ^= prototype_.hash();

		for (uint32_t i = 0; i < property_size_; ++i) {
			const auto& prop = properties_[i];
			hash_ = (hash_ ^ prop.const_index().value()) * 16777619;
			hash_ = (hash_ ^ prop.flags()) * 16777619;
		}
	}




	bool equal_new_shape(Shape* base_shape, ShapeProperty* property) {
		auto new_shape_prop_size = base_shape->property_size_ + 1;

		if (this == base_shape && property == nullptr) return false;
		if (property_size_ != new_shape_prop_size) return false;
		if (class_id_ != base_shape->class_id_) return false;
		if (prototype_ == base_shape->prototype_) return false;

		for (uint32_t i = 0; i < property_size_; ++i) {
			const auto& lhs_prop = properties_[i];
			const auto& rhs_prop = base_shape->properties_[i];

			if (lhs_prop.const_index().is_invalid() && rhs_prop.const_index().is_invalid()) {
				continue;
			}

			if (lhs_prop.flags() != rhs_prop.flags() ||
				lhs_prop.const_index() != rhs_prop.const_index()) {
				return false;
			}
		}

		const auto& lhs_prop = properties_[property_size_ - 1];
		const auto& rhs_prop = *property;

		if (lhs_prop.flags() != rhs_prop.flags() ||
			lhs_prop.const_index() != rhs_prop.const_index()) {
			return false;
		}

		return true;
	}

	static size_t calc_new_shape_hash(Shape* base_shape, const ShapeProperty& property) {
		size_t hash = base_shape->hash();
		hash = (hash ^ property.const_index().value()) * 16777619;
		hash = (hash ^ property.flags()) * 16777619;
		return hash;
	}



	size_t hash() const { return hash_; }

	uint32_t property_size() const { return property_size_; }

	const ShapeProperty* properties() const { return properties_; }

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


class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);
	~ShapeManager();

	Shape* add_property(Shape* base_shape, const ShapeProperty& property);

	Context& context() { return *context_; }

private:
	Context* context_;
	// ankerl::unordered_dense::set<Shape> shape_cache_;
};

} // namespace mjs
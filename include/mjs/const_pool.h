#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <mjs/noncopyable.h>
#include <mjs/segmented_array.h>
#include <mjs/const_def.h>
#include <mjs/value.h>

// ���������
// ȫ�ֳ���λ��ȫ�ֳ�����

// ����ʱ�����ĳ���λ������ʱ�����أ�ͨ�����ü���������

namespace mjs {

class GlobalConstPool : public SegmentedArray<Value, ConstIndex, 1024> {
private:
	using Base = SegmentedArray<Value, ConstIndex, 1024>;

public:
	ConstIndex insert(const Value& value) {
		auto value_ = value;
		return insert(std::move(value_));
	}

	ConstIndex insert(Value&& value) {
		auto lock = std::lock_guard(mutex_);

		auto it = map_.find(value);
		if (it != map_.end()) {
			return it->second;
		}

		auto idx = Base::insert(std::move(value));
		auto& val = operator[](idx);

		val.set_const_index(idx);
		auto res = map_.emplace(val, idx);

		idx = idx.to_global_index();
		return idx;
	}

	std::optional<ConstIndex> find(const Value& value) {
		auto lock = std::lock_guard(mutex_);
		auto it = map_.find(value);
		if (it != map_.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	const Value& operator[](ConstIndex index) const {
		return const_cast<GlobalConstPool*>(this)->operator[](index);
	}

	Value& operator[](ConstIndex index) {
		index.from_global_index();
		auto& val = Base::operator[](index);
		return val;
	}

	const Value& at(ConstIndex index) const {
		index.from_global_index();
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	Value& at(ConstIndex index) {
		index.from_global_index();
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	void clear() {
		map_.clear();
		Base::clear();
	}

private:
	std::mutex mutex_;
	std::unordered_map<Value, ConstIndex> map_;
};

class LocalConstPool : public noncopyable {
public:
	LocalConstPool();

	ConstIndex insert(const Value& value);
	ConstIndex insert(Value&& value);

	const Value& at(ConstIndex index) const;
	Value& at(ConstIndex index);

	const Value& operator[](ConstIndex index) const {
		return const_cast<LocalConstPool*>(this)->operator[](index);
	}

	Value& operator[](ConstIndex index) {
		index.from_local_index();
		auto& val = *pool_[index].value_;
		return val;
	}

	void ReferenceConst(ConstIndex index) {
		index.from_local_index();
		auto& node = pool_[index];
		++node.reference_count_;
	}

	void DereferenceConst(ConstIndex index) {
		index.from_local_index();
		auto& node = pool_[index];
		assert(node.reference_count_ > 0);
		--node.reference_count_;
		if (node.reference_count_ == 0) {
			erase(index);
		}
	}

private:
	void erase(ConstIndex index);

private:
	std::unordered_map<Value, ConstIndex> map_;

	size_t first_ = -1;
	struct Node {
		union {
			Value* value_;
			size_t next_;
		};
		uint32_t reference_count_ = 0;
	};
	std::vector<Node> pool_;
};

} // namespace mjs
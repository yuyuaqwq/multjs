#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <mjs/noncopyable.h>
#include <mjs/segmented_array.h>
#include <mjs/constant.h>
#include <mjs/value.h>

// 常量池设计
// 全局常量位于全局常量池
// 运行时创建的常量位于运行时常量池，通过引用计数来回收

namespace mjs {

class GlobalConstPool : public SegmentedArray<Value, ConstIndex, 1024> {
private:
	using Base = SegmentedArray<Value, ConstIndex, 1024>;

public:
	ConstIndex insert(const Value& value);

	ConstIndex insert(Value&& value);

	std::optional<ConstIndex> find(const Value& value);

	const Value& operator[](ConstIndex index) const {
		return const_cast<GlobalConstPool*>(this)->operator[](index);
	}

	Value& operator[](ConstIndex index) {
		auto& val = Base::operator[](index);
		return val;
	}

	const Value& at(ConstIndex index) const {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	Value& at(ConstIndex index) {
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

	std::optional<ConstIndex> find(const Value& value);

	const Value& at(ConstIndex index) const;
	Value& at(ConstIndex index);

	const Value& operator[](ConstIndex index) const {
		return const_cast<LocalConstPool*>(this)->operator[](index);
	}

	Value& operator[](ConstIndex index) {
		return *pool_[-index].value_;
	}

	void ReferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		++node.reference_count_;
	}

	void DereferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		assert(node.reference_count_ > 0);
		--node.reference_count_;
		if (node.reference_count_ == 0) {
			erase(index);
		}
	}

	void clear() {
		map_.clear();
		pool_.clear();
	}

private:
	void erase(ConstIndex index);

private:
	std::unordered_map<Value, ConstIndex> map_;

	int64_t first_ = -1;
	struct Node {
		union {
			Value* value_;
			int64_t next_;
		};
		uint32_t reference_count_ = 0;
	};
	std::vector<Node> pool_;
};

} // namespace mjs
#pragma once

#include <array>
#include <string>
#include <memory>
#include <optional>

#include <mjs/noncopyable.h>
// #include <mjs/const_def.h>

namespace mjs {

// ʹ�÷ֶξ�̬���飬����resizeʱ��������Context���߳�get������get��Ҫ����
// ÿ�龲̬������kStaticArraySize��Ԫ�أ����˾�new�µľ�̬����
template <typename T, typename IndexT, size_t kStaticArraySize>
class SegmentedArray : public noncopyable {
private:
	using StaticArray = std::array<T, kStaticArraySize>;

public:
	SegmentedArray() {
		pool_[0] = std::make_unique<StaticArray>();
	}

	IndexT insert(const T& value) {
		auto value_ = value;
		return insert(std::move(value_));
	}

	IndexT insert(T&& value) {
		if (size_ % kStaticArraySize == 0) {
			auto i1 = size_ / kStaticArraySize;
			if (i1 >= kStaticArraySize) {
				throw std::overflow_error("The number of constants exceeds the upper limit.");
			}
			pool_[i1] = std::make_unique<StaticArray>();
		}

		auto idx = size_;
		auto& val = operator[](idx);
		val = std::move(value);

		// �����++��������Ϊget�����������ﲻȷ���᲻�ᱻ���ŵ����棬�п�����Ҫʹ��ԭ��
		++size_;
		return idx;
	}

	const T& operator[](IndexT index) const {
		return const_cast<SegmentedArray*>(this)->operator[](index);
	}

	T& operator[](IndexT index) {
		// index = GlobalToConstIndex(index);
		auto i1 = index / kStaticArraySize;
		auto i2 = index % kStaticArraySize;
		return (*pool_[i1])[i2];
	}

	const T& at(IndexT index) const {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	T& at(IndexT index) {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	size_t size() {
		return size_;
	}

private:
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	IndexT size_ = 1;
};

} // namespace mjs
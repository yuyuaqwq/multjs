#pragma once

#include <array>
#include <string>
#include <memory>
#include <optional>

#include <mjs/noncopyable.h>
// #include <mjs/const_def.h>

namespace mjs {

// 使用分段静态数组，避免resize时其他运行Context的线程get，导致get需要加锁
// 每块静态数组有kStaticArraySize个元素，满了就new新的静态数组
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

		// 最后再++，但是因为get不加锁，这里不确定会不会被重排到上面，有可能需要使用原子
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

	void clear() {
		for (auto& ptr : pool_) {
			ptr.reset();
		}
	}

private:
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_;
	IndexT size_ = 1;
};

} // namespace mjs
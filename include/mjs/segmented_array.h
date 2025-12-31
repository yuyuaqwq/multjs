/**
 * @file segmented_array.h
 * @brief JavaScript 分段数组系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的分段数组系统，包括分段数组模板类，
 * 用于实现线程安全的数据存储结构，避免 resize 时的锁竞争。
 */

#pragma once

#include <array>
#include <string>
#include <memory>
#include <optional>

#include <mjs/noncopyable.h>
// #include <mjs/const_def.h>

namespace mjs {

/**
 * @class SegmentedArray
 * @brief 分段数组模板类
 *
 * 使用分段静态数组实现，避免 resize 时其他运行 Context 的线程 get，导致 get 需要加锁。
 * 每块静态数组有 kStaticArraySize 个元素，满了就 new 新的静态数组。
 * size初始为1，[0] 保留。
 *
 * @tparam T 元素类型
 * @tparam IndexT 索引类型
 * @tparam kStaticArraySize 静态数组大小
 * @see noncopyable 不可拷贝基类
 */
template <typename T, typename IndexT, size_t kStaticArraySize>
class SegmentedArray : public noncopyable {
private:
	using StaticArray = std::array<T, kStaticArraySize>;

public:
	/**
	 * @brief 默认构造函数
	 */
	SegmentedArray() {
		pool_[0] = std::make_unique<StaticArray>();
	}

	/**
	 * @brief 插入元素（常量引用版本）
	 * @param value 要插入的元素
	 * @return 插入位置的索引
	 */
	IndexT insert(const T& value) {
		auto value_ = value;
		return insert(std::move(value_));
	}

	/**
	 * @brief 插入元素（移动语义版本）
	 * @param value 要插入的元素
	 * @return 插入位置的索引
	 * @throw std::overflow_error 当常量数量超过上限时抛出
	 */
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

	/**
	 * @brief 常量下标访问运算符
	 * @param index 索引位置
	 * @return 常量元素引用
	 */
	const T& operator[](IndexT index) const {
		return const_cast<SegmentedArray*>(this)->operator[](index);
	}

	/**
	 * @brief 下标访问运算符
	 * @param index 索引位置
	 * @return 元素引用
	 */
	T& operator[](IndexT index) {
		// index = GlobalToConstIndex(index);
		auto i1 = index / kStaticArraySize;
		auto i2 = index % kStaticArraySize;
		return (*pool_[i1])[i2];
	}

	/**
	 * @brief 常量安全访问方法
	 * @param index 索引位置
	 * @return 常量元素引用
	 * @throw std::out_of_range 当索引超出范围时抛出
	 */
	const T& at(IndexT index) const {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	/**
	 * @brief 安全访问方法
	 * @param index 索引位置
	 * @return 元素引用
	 * @throw std::out_of_range 当索引超出范围时抛出
	 */
	T& at(IndexT index) {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	/**
	 * @brief 获取数组大小
	 * @return 当前元素数量
	 */
	size_t size() const {
		return size_;
	}

	/**
	 * @brief 清空数组
	 * 清空后size初始化为1
	 */
	void clear() {
		for (auto& ptr : pool_) {
			ptr.reset();
		}
		pool_[0] = std::make_unique<StaticArray>();
		size_ = IndexT(1);
	}

private:
	std::array<std::unique_ptr<StaticArray>, kStaticArraySize> pool_; ///< 分段数组池
	IndexT size_ = IndexT(1);                                         ///< 当前元素数量
};

} // namespace mjs
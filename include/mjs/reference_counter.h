/**
 * @file reference_counter.h
 * @brief JavaScript 引用计数系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的引用计数系统，包括引用计数模板类，
 * 用于实现自动内存管理和垃圾回收功能。
 */

#pragma once

#include <cstdint>

#include <mjs/noncopyable.h>

namespace mjs {

/**
 * @class ReferenceCounter
 * @brief 引用计数模板类
 *
 * 提供引用计数功能，支持自动内存回收。当引用计数降为0时自动删除对象。
 * 继承自 noncopyable，确保不可拷贝。
 *
 * @tparam T 对象类型
 * @note 避免循环引用问题
 * @see noncopyable 不可拷贝基类
 */
template <typename T>
class ReferenceCounter : public noncopyable {
public:
	/**
	 * @brief 默认构造函数
	 */
	ReferenceCounter() = default;

	/**
	 * @brief 析构函数
	 */
	~ReferenceCounter() = default;

	/**
	 * @brief 增加引用计数
	 */
	void Reference() {
		++ref_count_;
	}

	/**
	 * @brief 减少引用计数
	 *
	 * 当引用计数降为0时自动删除对象。
	 */
	void Dereference() {
		--ref_count_;
		if (ref_count_ == 0) {
			delete static_cast<T*>(this);
		}
	}

	/**
	 * @brief 获取引用计数值
	 * @return 当前引用计数值
	 */
	uint32_t ref_count() { return ref_count_; }

private:
	uint32_t ref_count_ = 0; ///< 引用计数值
};

} // namespace mjs
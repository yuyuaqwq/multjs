/**
 * @file string.h
 * @brief JavaScript 字符串类型定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的字符串类型，包括字符串的创建、
 * 格式化、哈希计算和内存管理功能。
 */

#pragma once

#include <string_view>
#include <format>

#include <mjs/reference_counter.h>

namespace mjs {

/**
 * @class String
 * @brief JavaScript 字符串类
 *
 * 提供 JavaScript 字符串的完整功能，包括：
 * - 引用计数内存管理
 * - 字符串格式化
 * - 哈希计算和缓存
 * - 灵活的内存分配
 *
 * 使用柔性数组存储字符串数据，实现零拷贝字符串操作。
 *
 * @note 不会有循环引用问题，仅使用引用计数管理
 * @warning 字符串对象使用引用计数，需要正确管理引用
 * @see ReferenceCounter 引用计数基类
 */
class String : public ReferenceCounter<String> {
private:
	/**
	 * @brief 私有构造函数
	 * @param size 字符串长度
	 */
	String(size_t size)
		: size_(size) {}

public:
	/**
	 * @brief 获取字符串哈希值
	 * @return 字符串哈希值
	 */
	size_t hash() const {
		return hash_;
	}

	/**
	 * @brief 获取字符串数据指针
	 * @return 字符串数据常量指针
	 */
	const char* data() const {
		return data_;
	}

	/**
	 * @brief 检查字符串是否为空
	 * @return 是否为空字符串
	 */
	bool empty() const {
		return size_ == 0;
	}

	/**
	 * @brief 格式化字符串
	 *
	 * 使用 std::format 语法创建格式化字符串，支持类型安全的字符串格式化。
	 *
	 * @tparam Args 格式化参数类型
	 * @param fmt 格式化字符串
	 * @param args 格式化参数
	 * @return 格式化后的字符串指针
	 * @note 使用 placement new 和柔性数组实现高效内存分配
	 */
	template<typename... Args>
	static String* Format(std::format_string<Args...> fmt, Args&&... args) {
		// First, format into a temporary buffer to determine the size
		const auto size = std::formatted_size(fmt, std::forward<Args>(args)...);

		// Allocate memory for String object plus space for the formatted string
		String* s = static_cast<String*>(::operator new(sizeof(String) + size + 1));

		// Use placement new to construct the String object
		new (s) String(size);

		// Format directly into our data_ buffer
		std::format_to_n(s->data_, size + 1, fmt, std::forward<Args>(args)...);
		s->data_[size] = '\0';  // Ensure null-termination

		// Calculate hash based on the formatted content
		std::string_view view(s->data_, size);
		s->hash_ = std::hash<std::string_view>()(view);

		return s;
	}

	/**
	 * @brief 从字符串视图创建字符串
	 * @param str 字符串视图
	 * @return 新创建的字符串指针
	 */
	static String* New(std::string_view str) {
		auto size = str.size();
		// Allocate memory for String object plus space for the string data
		String* s = static_cast<String*>(::operator new(sizeof(String) + size + 1));
		// Use placement new to construct the String object
		new (s) String(size);
		// Copy the string data
		memcpy(s->data_, str.data(), size);
		s->data_[size] = '\0';  // Ensure null-termination

		// Calculate the hash based on the actual string content
		s->hash_ = std::hash<std::string_view>()(str);
		return s;
	}

	/**
	 * @brief 从迭代器范围创建字符串
	 * @tparam Iterator 迭代器类型
	 * @param begin 起始迭代器
	 * @param end 结束迭代器
	 * @return 新创建的字符串指针
	 */
	template <typename Iterator>
	static String* New(Iterator begin, Iterator end) {
		return New(std::string_view(begin, end));
	}

private:
	size_t hash_ = 0;   ///< 字符串哈希值
	size_t size_;       ///< 字符串长度
	char data_[];       ///< 字符串数据（柔性数组）
};

} // namespace mjs
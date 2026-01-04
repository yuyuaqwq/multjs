/**
 * @file shape_property.h
 * @brief JavaScript 形状属性定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的形状属性类，用于表示对象的属性信息。
 */

#pragma once

#include <mjs/value.h>

namespace mjs {

/**
 * @class ShapeProperty
 * @brief 形状属性类
 *
 * 表示对象的属性信息，包括属性标志和常量索引。
 * 用于形状系统中的属性管理。
 *
 * 属性描述符系统：
 * - 支持数据属性和访问器属性（getter/setter）
 * - 支持标准的属性特性（enumerable, configurable, writable）
 */
class ShapeProperty {
public:
	// 属性类型标志
	enum Flags {
		// Accessor 属性标志
		kNone = 0,
		kIsGetter = 1 << 0,   ///< 是 getter
		kIsSetter = 1 << 1,   ///< 是 setter

		// 标准属性特性（JS Property Attributes）
		kEnumerable = 1 << 2,   ///< 可枚举
		kConfigurable = 1 << 3, ///< 可配置
		kWritable = 1 << 4,     ///< 可写（仅数据属性）

		// 便捷组合标志
		kDefault = kEnumerable | kConfigurable | kWritable,  ///< 默认属性特性
		kReadOnly = kEnumerable | kConfigurable,            ///< 只读属性特性
	};

	ShapeProperty() = default;
	ShapeProperty(uint32_t flags, ConstIndex const_index);
	~ShapeProperty() = default;

	uint32_t flags() const { return flags_; }
	void set_flags(uint32_t flags) { flags_ = flags; }

	ConstIndex const_index() const { return const_index_; }
	void set_const_index(ConstIndex const_index) { const_index_ = const_index; }

	// Accessor 属性检查
	bool is_getter() const { return flags_ & kIsGetter; }
	bool is_setter() const { return flags_ & kIsSetter; }
	bool is_accessor() const { return is_getter() || is_setter(); }

	// 属性特性检查
	bool is_enumerable() const { return flags_ & kEnumerable; }
	bool is_configurable() const { return flags_ & kConfigurable; }
	bool is_writable() const { return flags_ & kWritable; }

	// 辅助方法：检查是否为数据属性（非 accessor）
	bool is_data_property() const { return !is_accessor(); }

private:
	uint32_t flags_ = 0;
	ConstIndex const_index_;
};

} // namespace mjs
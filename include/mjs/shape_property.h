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
 */
class ShapeProperty {
public:
	ShapeProperty() = default;
	ShapeProperty(uint32_t flags, ConstIndex const_index);
	~ShapeProperty() = default;

	uint32_t flags() const { return flags_; }
	void set_flags(uint32_t flags) { flags_ = flags; }

	ConstIndex const_index() const { return const_index_; }
	void set_const_index(ConstIndex const_index) { const_index_ = const_index; }

private:
	uint32_t flags_ = 0;
	ConstIndex const_index_;
};

} // namespace mjs
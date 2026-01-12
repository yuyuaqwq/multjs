/**
 * @file shape_manager.h
 * @brief JavaScript 形状管理器定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的形状管理器类，用于管理形状的创建和属性添加。
 */

#pragma once

#include <mjs/shape.h>

namespace mjs {

class Context;

/**
 * @class ShapeManager
 * @brief 形状管理器类
 *
 * 管理形状的创建和属性添加，提供形状系统的统一管理接口。
 * 继承自 noncopyable，确保不可拷贝。
 *
 * @note 后续的修改方向是，引用计数归0不会回收对象
 * @note 只有 GC 的时候，递归到没有子节点的节点，且该节点引用计数为0，才能清除当前节点
 * @see noncopyable 不可拷贝基类
 */
class ShapeManager : public noncopyable {
public:
	ShapeManager(Context* context);

	~ShapeManager();

	PropertySlotIndex AddProperty(Shape** base_shape, ShapeProperty&& property);

	auto& context() { return *context_; }

	Shape& empty_shape() { return *empty_shape_; }

private:
	Context* context_;
	Shape* empty_shape_;
};

} // namespace mjs
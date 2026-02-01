/**
 * @file transition_table.h
 * @brief JavaScript 过渡表定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的过渡表类，用于管理形状之间的过渡关系。
 */

#pragma once

#include <mjs/unordered_dense.h>
#include <mjs/constant.h>

namespace mjs {

class Shape;

/**
 * @class TransitionTable
 * @brief 过渡表类
 *
 * 管理形状之间的过渡关系，支持形状查找、添加和删除操作。
 * 过渡表不引用 ConstIndex，因为有过渡表就说明有属性到子 Shape，
 * 子 Shape 的 PropertyMap 中引用一份就行。
 *
 * @note 过渡表不引用 ConstIndex
 * @note 因为有过渡表就说明有属性到子 Shape，子 Shape 的 PropertyMap 中引用一份就行
 * @note Shape 释放的时候也会从父 Shape 的过渡表中删除
 */
class TransitionTable {
public:
	TransitionTable() = default;

	~TransitionTable();

	bool Has() const;

	Shape* Find(ConstIndex key) const;

	void Add(ConstIndex key, Shape*);

	bool Delete(ConstIndex key);

private:
	enum class Type {
		kNone,
		kOne,
		kMap
	} type_ = Type::kNone;

	ConstIndex key_;
	
	union {
		Shape* shape_;
		ankerl::unordered_dense::map<ConstIndex, Shape*>* map_;
	};
};

} // namespace mjs
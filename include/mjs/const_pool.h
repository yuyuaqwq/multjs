/**
 * @file const_pool.h
 * @brief JavaScript 常量池系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的常量池系统，包括全局常量池和本地常量池。
 * 全局常量位于全局常量池，运行时创建的常量位于运行时常量池，通过引用计数来回收。
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <mjs/noncopyable.h>
#include <mjs/segmented_array.h>
#include <mjs/constant.h>
#include <mjs/value.h>

namespace mjs {

/**
 * @class GlobalConstPool
 * @brief 全局常量池类
 *
 * 继承自 SegmentedArray，提供全局常量的存储和管理功能：
 * - 常量插入和查找
 * - 线程安全访问
 * - 常量索引管理
 *
 * @note 全局常量池在整个运行时生命周期内存在
 * @see SegmentedArray 分段数组基类
 * @see LocalConstPool 本地常量池类
 */
class GlobalConstPool : public SegmentedArray<Value, ConstIndex, 1024> {
private:
	using Base = SegmentedArray<Value, ConstIndex, 1024>;

public:
	/**
	 * @brief 插入常量值
	 * @param value 常量值
	 * @return 常量索引
	 */
	ConstIndex insert(const Value& value);

	/**
	 * @brief 插入常量值（移动语义）
	 * @param value 常量值
	 * @return 常量索引
	 */
	ConstIndex insert(Value&& value);

	/**
	 * @brief 查找常量值
	 * @param value 常量值
	 * @return 常量索引（如果存在），否则返回空
	 */
	std::optional<ConstIndex> find(const Value& value);

	/**
	 * @brief 常量索引访问运算符（常量版本）
	 * @param index 常量索引
	 * @return 常量值常量引用
	 */
	const Value& operator[](ConstIndex index) const {
		return const_cast<GlobalConstPool*>(this)->operator[](index);
	}

	/**
	 * @brief 常量索引访问运算符
	 * @param index 常量索引
	 * @return 常量值引用
	 */
	Value& operator[](ConstIndex index) {
		auto& val = Base::operator[](index);
		return val;
	}

	/**
	 * @brief 安全访问常量值（常量版本）
	 * @param index 常量索引
	 * @return 常量值常量引用
	 * @throw std::out_of_range 当索引超出范围时抛出
	 */
	const Value& at(ConstIndex index) const {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	/**
	 * @brief 安全访问常量值
	 * @param index 常量索引
	 * @return 常量值引用
	 * @throw std::out_of_range 当索引超出范围时抛出
	 */
	Value& at(ConstIndex index) {
		if (index < 0 || index >= size()) {
			throw std::out_of_range("Index out of range");
		}
		return (*this)[index];
	}

	/**
	 * @brief 清空常量池
	 */
	void clear() {
		map_.clear();
		Base::clear();
	}

private:
	std::mutex mutex_;                       ///< 互斥锁，保证线程安全
	std::unordered_map<Value, ConstIndex> map_; ///< 值到索引的映射表
};

/**
 * @class LocalConstPool
 * @brief 本地常量池类
 *
 * 提供运行时创建的常量的存储和管理功能：
 * - 引用计数管理
 * - 自动内存回收
 * - 常量索引管理
 *
 * @note 本地常量池通过引用计数来回收内存
 * @see GlobalConstPool 全局常量池类
 */
class LocalConstPool : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 */
	LocalConstPool();

	/**
	 * @brief 插入常量值
	 * @param value 常量值
	 * @return 常量索引
	 */
	ConstIndex insert(const Value& value);

	/**
	 * @brief 插入常量值（移动语义）
	 * @param value 常量值
	 * @return 常量索引
	 */
	ConstIndex insert(Value&& value);

	/**
	 * @brief 查找常量值
	 * @param value 常量值
	 * @return 常量索引（如果存在），否则返回空
	 */
	std::optional<ConstIndex> find(const Value& value);

	/**
	 * @brief 安全访问常量值（常量版本）
	 * @param index 常量索引
	 * @return 常量值常量引用
	 */
	const Value& at(ConstIndex index) const;

	/**
	 * @brief 安全访问常量值
	 * @param index 常量索引
	 * @return 常量值引用
	 */
	Value& at(ConstIndex index);

	/**
	 * @brief 常量索引访问运算符（常量版本）
	 * @param index 常量索引
	 * @return 常量值常量引用
	 */
	const Value& operator[](ConstIndex index) const {
		return const_cast<LocalConstPool*>(this)->operator[](index);
	}

	/**
	 * @brief 常量索引访问运算符
	 * @param index 常量索引
	 * @return 常量值引用
	 */
	Value& operator[](ConstIndex index) {
		return *pool_[-index].value_;
	}

	/**
	 * @brief 增加常量引用计数
	 * @param index 常量索引
	 */
	void ReferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		++node.reference_count_;
	}

	/**
	 * @brief 减少常量引用计数
	 * @param index 常量索引
	 * @note 当引用计数为0时自动回收常量
	 */
	void DereferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		assert(node.reference_count_ > 0);
		--node.reference_count_;
		if (node.reference_count_ == 0) {
			erase(index);
		}
	}

	/**
	 * @brief 清空常量池
	 */
	void clear() {
		map_.clear();
		pool_.clear();
	}

private:
	/**
	 * @brief 删除常量
	 * @param index 常量索引
	 */
	void erase(ConstIndex index);

private:
	std::unordered_map<Value, ConstIndex> map_; ///< 值到索引的映射表

	int64_t first_ = -1;                        ///< 第一个空闲节点索引
	struct Node {
		union {
			Value* value_;                          ///< 常量值指针
			int64_t next_;                         ///< 下一个空闲节点索引
		};
		uint32_t reference_count_ = 0;           ///< 引用计数
	};
	std::vector<Node> pool_;                    ///< 节点池
};

} // namespace mjs
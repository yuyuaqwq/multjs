/**
 * @file global_const_pool.h
 * @brief JavaScript 常量池系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的局部常量池。
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <optional>

#include <mjs/noncopyable.h>
#include <mjs/constant.h>
#include <mjs/value/value.h>

namespace mjs {

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
	ConstIndex FindOrInsert(const Value& value);

	/**
	 * @brief 插入常量值（移动语义）
	 * @param value 常量值
	 * @return 局部常量索引（负数）
	 */
	ConstIndex FindOrInsert(Value&& value);

	/**
	 * @brief 查找常量值
	 * @param value 常量值
	 * @return 局部常量索引（负数，如果存在），否则返回空
	 */
	std::optional<ConstIndex> Find(const Value& value);

	/**
	 * @brief 安全访问常量值（常量版本）
	 * @param index 局部常量索引
	 * @return 常量值常量引用
	 */
	const Value& At(ConstIndex index) const;

	/**
	 * @brief 安全访问常量值
	 * @param index 局部常量索引
	 * @return 常量值引用
	 */
	Value& At(ConstIndex index);

	/**
	 * @brief 常量索引访问运算符（常量版本）
	 * @param index 局部常量索引
	 * @return 常量值常量引用
	 */
	const Value& operator[](ConstIndex index) const {
		return const_cast<LocalConstPool*>(this)->operator[](index);
	}

	/**
	 * @brief 常量索引访问运算符
	 * @param index 局部常量索引
	 * @return 常量值引用
	 */
	Value& operator[](ConstIndex index) {
		return *pool_[-index].value_;
	}

	/**
	 * @brief 增加常量引用计数
	 * @param index 局部常量索引
	 */
	void ReferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		++node.reference_count_;
	}

	/**
	 * @brief 减少常量引用计数
	 * @param index 局部常量索引
	 * @note 当引用计数为0时自动回收常量
	 */
	void DereferenceConst(ConstIndex index) {
		auto& node = pool_[-index];
		assert(node.reference_count_ > 0);
		--node.reference_count_;
		if (node.reference_count_ == 0) {
			Erase(index);
		}
	}

	/**
	 * @brief 清空常量池
	 */
	void Clear() {
		map_.clear();
		pool_.clear();
	}

private:
	/**
	 * @brief 删除常量
	 * @param index 局部常量索引
	 */
	void Erase(ConstIndex index);

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
/**
 * @file key_const_index_table.h
 * @brief JavaScript 预设Key常量索引表
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的预设Key常量索引表。
 */

#pragma once

#include <mjs/noncopyable.h>
#include <mjs/global_const_pool.h>

namespace mjs {

/**
 * @class KeyConstIndexTable
 * @brief 预设Key常量索引表
 */
class KeyConstIndexTable : public noncopyable {
public:
    KeyConstIndexTable(GlobalConstPool* global_const_pool);

    ~KeyConstIndexTable();

	ConstIndex constructor_const_index() const { return constructor_const_index_; }

	ConstIndex prototype_const_index() const { return prototype_const_index_; }

private:
	ConstIndex constructor_const_index_;
	ConstIndex prototype_const_index_;
};

} // namespace mjs
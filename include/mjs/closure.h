/**
 * @file closure.h
 * @brief JavaScript 闭包系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的闭包系统，包括闭包变量、闭包环境
 * 和闭包变量表等组件，用于支持词法作用域和闭包功能。
 */

#pragma once

#include <mjs/reference_counter.h>
#include <mjs/object.h>

namespace mjs {

/**
 * @class ClosureVar
 * @brief 闭包变量类
 *
 * 表示提升到堆的变量，用于实现闭包功能。继承自 ReferenceCounter
 * 提供引用计数管理，支持自动内存回收。
 *
 * @note 避免循环引用问题
 * @see ReferenceCounter 引用计数基类
 */
class ClosureVar : public ReferenceCounter<ClosureVar> {
public:
	/**
	 * @brief 构造函数
	 * @param value 变量值
	 */
	ClosureVar(Value&& value)
		: value_(std::move(value))
	{
		assert(!value_.IsClosureVar());
	}

	/**
	 * @brief 析构函数
	 */
	~ClosureVar() = default;

	/**
	 * @brief 获取变量值引用
	 * @return 变量值引用
	 */
	Value& value() { return value_; }

	/**
	 * @brief 获取变量值常量引用
	 * @return 变量值常量引用
	 */
	const Value& value() const { return value_; }

public:
	Value value_; ///< 变量值
};

/**
 * @class ClosureEnvironment
 * @brief 闭包环境记录类
 *
 * 存储闭包捕获的变量引用和词法作用域的 this 值。
 * 用于实现闭包的词法作用域功能。
 *
 * @note 也可以改成 ClosureVar*，手动调用 Reference 和 Dereference，可以节省一些空间
 */
class ClosureEnvironment {
public:
	/**
	 * @brief 垃圾回收遍历子对象
	 * @param context 执行上下文指针
	 * @param list 对象链表
	 * @param callback 回调函数，用于标记子对象
	 */
	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		for (auto& var : closure_var_refs_) {
			callback(context, list, var);
		}
		callback(context, list, lexical_this_);
	}

	/**
	 * @brief 获取闭包变量引用常量引用
	 * @return 闭包变量引用常量引用
	 */
	const auto& closure_var_refs() const { return closure_var_refs_; }

	/**
	 * @brief 获取闭包变量引用引用
	 * @return 闭包变量引用引用
	 */
	auto& closure_var_refs() { return closure_var_refs_; }

	/**
	 * @brief 获取词法作用域 this 值常量引用
	 * @return 词法作用域 this 值常量引用
	 */
	const auto& lexical_this() const { return lexical_this_; }

	/**
	 * @brief 设置词法作用域 this 值
	 * @param lexical_this 词法作用域 this 值
	 */
	void set_lexical_this(Value&& lexical_this) { lexical_this_ = lexical_this; }

private:
	std::vector<Value> closure_var_refs_; ///< 闭包变量引用向量
	Value lexical_this_;                  ///< 词法作用域捕获的 this 值
};

/**
 * @struct ClosureVarDef
 * @brief 闭包变量定义结构体
 *
 * 定义闭包变量的相关信息，包括环境变量索引和父作用域变量索引。
 */
struct ClosureVarDef {
	uint32_t env_var_idx;     ///< 该变量在 ClosureEnvironment::vars_ 的索引
	VarIndex parent_var_idx;  ///< 在父作用域中的变量索引
};

/**
 * @class ClosureVarTable
 * @brief 闭包变量表类
 *
 * 管理闭包变量的定义信息，提供闭包变量的添加和查询功能。
 * 用于记录捕获的外部变量信息。
 */
class ClosureVarTable {
public:
	/**
	 * @brief 添加闭包变量
	 * @param var_idx 变量索引
	 * @param parent_var_idx 父作用域变量索引
	 */
	void AddClosureVar(VarIndex var_idx, VarIndex parent_var_idx) {
		closure_var_defs_.emplace(var_idx,
			ClosureVarDef{
				.env_var_idx = uint32_t(closure_var_defs_.size()),
				.parent_var_idx = parent_var_idx,
			}
		);
	}

	/**
	 * @brief 获取闭包变量定义引用
	 * @return 闭包变量定义引用
	 */
	auto& closure_var_defs() { return closure_var_defs_; }

	/**
	 * @brief 获取闭包变量定义常量引用
	 * @return 闭包变量定义常量引用
	 */
	const auto& closure_var_defs() const { return closure_var_defs_; }

private:
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_; ///< 捕获的外部变量定义表
};

} // namespace mjs
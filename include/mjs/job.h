/**
 * @file job.h
 * @brief JavaScript 作业系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的作业系统，包括作业类和相关功能，
 * 用于支持异步任务执行和垃圾回收遍历。
 */

#pragma once

#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>

namespace mjs {

/**
 * @class Job
 * @brief 作业类
 *
 * 表示一个待执行的 JavaScript 作业，包含函数、this 值和参数列表。
 * 作业在执行完成后统一释放，支持垃圾回收遍历子对象。
 *
 * @note 作业统一在执行后释放
 * @see noncopyable 不可拷贝基类
 */
class Job : public noncopyable {
public:
	/**
	 * @brief 构造函数
	 * @param func 作业函数
	 * @param this_val 作业 this 值
	 */
	Job(Value func, Value this_val)
		: func_(std::move(func))
		, this_val_(std::move(this_val)) {}

	/**
	 * @brief 移动构造函数
	 * @param other 要移动的作业对象
	 */
	Job(Job&& other) noexcept {
		operator=(std::move(other));
	}

	/**
	 * @brief 垃圾回收遍历子对象（旧接口）
	 * @param context 执行上下文指针
	 * @param list 对象链表
	 * @param callback 回调函数，用于标记子对象
	 */
	void ForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		callback(context, list, func_);
		callback(context, list, this_val_);
		for (auto& val : argv_) {
			callback(context, list, val);
		}
	}

	/**
	 * @brief 垃圾回收遍历子对象（新接口）
	 * @param context 执行上下文指针
	 * @param callback 回调函数，用于处理每个子对象引用
	 */
	void GCTraverse(Context* context, std::function<void(Context* ctx, Value& value)> callback) {
		callback(context, func_);
		callback(context, this_val_);
		for (auto& val : argv_) {
			callback(context, val);
		}
	}

	/**
	 * @brief 移动赋值运算符
	 * @param other 要移动的作业对象
	 */
	void operator=(Job&& other) noexcept {
		func_ = std::move(other.func_);
		this_val_ = std::move(other.this_val_);
		argv_ = std::move(other.argv_);
	}

	/**
	 * @brief 添加参数
	 * @param value 参数值
	 */
	void AddArg(Value value) {
		argv_.emplace_back(std::move(value));
	}

	/**
	 * @brief 获取作业函数常量引用
	 * @return 作业函数常量引用
	 */
	const auto& func() const { return func_; }

	/**
	 * @brief 获取作业函数引用
	 * @return 作业函数引用
	 */
	auto& func() { return func_; }

	/**
	 * @brief 获取作业 this 值常量引用
	 * @return 作业 this 值常量引用
	 */
	const auto& this_val() const { return this_val_; }

	/**
	 * @brief 获取参数列表常量引用
	 * @return 参数列表常量引用
	 */
	const auto& argv() const { return argv_; }

private:
	Value func_;                    ///< 作业函数
	Value this_val_;                ///< 作业 this 值
	std::vector<Value> argv_;       ///< 参数列表
};

} // namespace mjs
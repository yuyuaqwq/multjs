/**
 * @file object.h
 * @brief JavaScript 对象系统基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中所有对象类型的基类，提供了对象属性管理、
 * 垃圾回收、原型继承等核心功能。
 */

#pragma once

#include <string_view>
#include <map>
#include <unordered_map>
#include <functional>

#include <mjs/noncopyable.h>
#include <mjs/intrusive_list.hpp>
#include <mjs/constant.h>
#include <mjs/value.h>
#include <mjs/class_def.h>

namespace mjs {

class Shape;

/**
 * @class Object
 * @brief JavaScript 对象基类
 *
 * 所有 JavaScript 对象类型的基类，提供以下核心功能：
 * - 属性管理（设置、获取、删除属性）
 * - 引用计数和垃圾回收
 * - 原型继承支持
 * - 形状管理（对象布局优化）
 *
 * @note 保持和 JavaScript 标准一致，属性键只能是 string 或者 symbol
 * @warning 其他类型不会自动转换为 string，而是抛出异常
 * @see Value JavaScript 值类型
 * @see ClassDef 类定义
 */
class Object
	: public noncopyable
	, public intrusive_list<Object>::node {

protected:
	/**
	 * @brief 受保护构造函数
	 * @param runtime 运行时环境指针
	 * @param class_id 类标识符
	 */
	Object(Runtime* runtime, ClassId class_id);

	/**
	 * @brief 受保护构造函数
	 * @param context 执行上下文指针
	 * @param class_id 类标识符
	 */
	Object(Context* context, ClassId class_id);

public:
	/**
	 * @brief 虚析构函数
	 */
	virtual ~Object();

	/**
	 * @brief 增加对象引用计数
	 */
	virtual void Reference();

	/**
	 * @brief 减少对象引用计数
	 * @note 当引用计数为0时自动销毁对象
	 */
	void Dereference();

	/**
	 * @brief 弱引用减少
	 * 用于弱引用场景，不触发对象销毁
	 */
	void WeakDereference();

	/**
	 * @brief 垃圾回收遍历子对象
	 *
	 * 遍历对象中所有包含的 Value 子对象，用于垃圾回收标记阶段。
	 *
	 * @param context 执行上下文指针
	 * @param list 对象链表
	 * @param callback 回调函数，用于标记子对象
	 * @note 数据成员中有 Value，必须重写此方法，否则会导致内存泄漏
	 */
	virtual void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		for (auto& val : values_) {
			callback(context, list, val);
		}
	}

	/**
	 * @brief 设置对象属性（字符串键）
	 * @param runtime 运行时环境指针
	 * @param key 属性键名（字符串）
	 * @param value 属性值
	 */
	void SetProperty(Runtime* runtime, const char* key, Value&& value);

	/**
	 * @brief 获取对象属性（字符串键）
	 * @param runtime 运行时环境指针
	 * @param key 属性键名（字符串）
	 * @param value 输出参数，存储获取的属性值
	 * @return 是否成功获取属性
	 */
	bool GetProperty(Runtime* runtime, const char* key, Value* value);

	/**
	 * @brief 设置对象属性（常量索引键）
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 * @param value 属性值
	 */
	virtual void SetProperty(Context* context, ConstIndex key, Value&& value);

	/**
	 * @brief 获取对象属性（常量索引键）
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 * @param value 输出参数，存储获取的属性值
	 * @return 是否成功获取属性
	 */
	virtual bool GetProperty(Context* context, ConstIndex key, Value* value);

	/**
	 * @brief 检查对象是否包含指定属性
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 * @return 是否包含该属性
	 */
	virtual bool HasProperty(Context* context, ConstIndex key);

	/**
	 * @brief 删除对象属性
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 */
	virtual void DelProperty(Context* context, ConstIndex key);

	/**
	 * @brief 设置计算属性（动态键）
	 * @param context 执行上下文指针
	 * @param key 属性键值
	 * @param val 属性值
	 */
	virtual void SetComputedProperty(Context* context, const Value& key, Value&& val);

	/**
	 * @brief 获取计算属性（动态键）
	 * @param context 执行上下文指针
	 * @param key 属性键值
	 * @param value 输出参数，存储获取的属性值
	 * @return 是否成功获取属性
	 */
	virtual bool GetComputedProperty(Context* context, const Value& key, Value* value);

	/**
	 * @brief 删除计算属性（动态键）
	 * @param context 执行上下文指针
	 * @param key 属性键值
	 */
	virtual void DelComputedProperty(Context* context, const Value& key);

	/**
	 * @brief 将对象转换为字符串表示
	 * @param context 执行上下文指针
	 * @return 对象的字符串表示
	 */
	virtual Value ToString(Context* context);

	/**
	 * @brief 获取对象的原型
	 * @param runtime 运行时环境指针
	 * @return 原型对象的常量引用
	 */
	const Value& GetPrototype(Runtime* runtime) const;

	/**
	 * @brief 获取对象的类标识符
	 * @return 类标识符
	 */
	ClassId class_id() const { return static_cast<ClassId>(tag_.class_id_); }

	/**
	 * @brief 获取对象的类定义
	 * @param runtime 运行时环境指针
	 * @return 类定义引用
	 */
	ClassDef& GetClassDef(Runtime* runtime) const;

	/**
	 * @brief 获取指定类型的类定义
	 * @tparam ClassDefT 类定义类型
	 * @param runtime 运行时环境指针
	 * @return 指定类型的类定义常量引用
	 */
	template <typename ClassDefT>
	const ClassDefT& GetClassDef(Runtime* runtime) const {
		return static_cast<ClassDefT&>(GetClassDef(runtime));
	}

	/**
	 * @brief 获取指定类型的对象常量引用
	 * @tparam ObjectT 对象类型
	 * @return 指定类型的对象常量引用
	 */
	template <typename ObjectT>
	const ObjectT& get() const {
		return static_cast<ObjectT&>(*this);
	}

	/**
	 * @brief 获取指定类型的对象引用
	 * @tparam ObjectT 对象类型
	 * @return 指定类型的对象引用
	 */
	template <typename ObjectT>
	ObjectT& get() {
		return static_cast<ObjectT&>(*this);
	}

	/**
	 * @brief 获取对象的引用计数
	 * @return 当前引用计数
	 */
	auto ref_count() const { return tag_.ref_count_; }

	/**
	 * @brief 获取垃圾回收标记状态
	 * @return 垃圾回收标记状态
	 */
	bool gc_mark() { return  tag_.gc_mark_; }

	/**
	 * @brief 设置垃圾回收标记状态
	 * @param flag 标记状态
	 */
	void set_gc_mark(bool flag) { tag_.gc_mark_ = flag; }

	/**
	 * @brief 创建新的普通对象
	 * @param runtime 运行时环境指针
	 * @return 新创建的对象指针
	 */
	static Object* New(Runtime* runtime) {
		return new Object(runtime, ClassId::kObject);
	}

	/**
	 * @brief 创建新的普通对象
	 * @param context 执行上下文指针
	 * @return 新创建的对象指针
	 */
	static Object* New(Context* context) {
		return new Object(context, ClassId::kObject);
	}

protected:
	union {
		uint64_t full_ = 0;                   ///< 完整64位值
		struct {
			uint32_t ref_count_;                ///< 引用计数
			uint32_t gc_mark_ : 1;              ///< 垃圾回收标记位
			uint32_t is_const_ : 1;             ///< 是否为常量对象标记
			uint32_t class_id_ : 16;            ///< 类标识符（16位）
		};
	} tag_;
	Shape* shape_;                          ///< 形状指针（对象布局描述）
	std::vector<Value> values_;             ///< 属性值向量
};

} // namespace mjs
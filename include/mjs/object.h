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
#include <mjs/shape_property.h>
#include <mjs/shape_property_hash_table.h>

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
	virtual void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child));

	/**
	 * @brief 设置对象属性（字符串键）
	 * @param runtime 运行时环境指针
	 * @param key 属性键名（字符串）
	 * @param value 属性值
	 */
	void SetProperty(Runtime* runtime, ConstIndex key, Value&& value);

	/**
	 * @brief 获取对象属性（字符串键）
	 * @param runtime 运行时环境指针
	 * @param key 属性键名（字符串）
	 * @param value 输出参数，存储获取的属性值
	 * @return 是否成功获取属性
	 */
	bool GetProperty(Runtime* runtime, ConstIndex key, Value* value);

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
	 * @brief 设置带标志的属性（用于 getter/setter）
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 * @param value 属性值
	 * @param flags 属性标志（如 kIsGetter/kIsSetter）
	 */
	void SetPropertyWithFlags(Context* context, ConstIndex key, Value&& value, uint32_t flags);

	/**
	 * @brief 定义 accessor 属性（getter/setter）
	 *
	 * 便捷方法，类似 QuickJS 的 JS_DefinePropertyGetSet
	 * 用于同时定义 getter 和 setter（或其中之一）
	 *
	 * @param context 执行上下文指针
	 * @param key 属性键索引
	 * @param getter getter 函数（可以是 nullptr）
	 * @param setter setter 函数（可以是 nullptr）
	 * @param flags 属性标志（enumerable, configurable 等）
	 */
	void DefineAccessorProperty(Context* context, ConstIndex key,
	                              FunctionObject* getter,
	                              FunctionObject* setter,
	                              uint32_t flags = ShapeProperty::kDefault);

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
	 * @return 原型对象的常量引用，__proto__
	 */
	const Value& GetPrototype(Runtime* runtime) const;

	/**
	 * @brief 设置对象的原型
	 * @param context 执行上下文指针
	 * @param prototype 要设置的原型对象，__proto__
	 */
	void SetPrototype(Context* context, Value prototype);

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

	/**
	 * @brief 冻结对象，使其不可修改（符合 JavaScript 标准）
	 *
	 * 冻结后的对象：
	 * 1. 不能添加新属性
	 * 2. 不能删除现有属性
	 * 3. 不能修改现有属性的值
	 * 4. 将所有属性设置为不可写（writable=false）和不可配置（configurable=false）
	 * 5. 对象变为不可扩展（extensible=false）
	 */
	void Freeze();

	/**
	 * @brief 检查对象是否已被冻结
	 * @return 是否已冻结
	 */
	bool IsFrozen() const;

	/**
	 * @brief 密封对象（符合 JavaScript 标准）
	 *
	 * 密封后的对象：
	 * 1. 不能添加新属性
	 * 2. 不能删除现有属性
	 * 3. 可以修改现有属性的值（如果可写）
	 * 4. 将所有属性设置为不可配置（configurable=false）
	 */
	void Seal();

	/**
	 * @brief 检查对象是否已被密封
	 * @return 是否已密封
	 */
	bool IsSealed() const;

	/**
	 * @brief 阻止对象扩展（符合 JavaScript 标准）
	 *
	 * 阻止扩展后的对象：
	 * 1. 不能添加新属性
	 * 2. 可以删除现有属性
	 * 3. 可以修改现有属性的值
	 */
	void PreventExtensions();

	/**
	 * @brief 检查对象是否可扩展
	 * @return 是否可扩展
	 */
	bool IsExtensible() const;

	/**
	 * @brief 获取指定索引的属性标志（对象独立的副本）
	 * @param index 属性索引
	 * @return 属性标志
	 */
	uint32_t GetPropertyFlags(PropertySlotIndex index) const {
		if (index >= 0 && index < static_cast<PropertySlotIndex>(properties_.size())) {
			return properties_[index].flags;
		}
		return ShapeProperty::kDefault;
	}

	/**
	 * @brief 设置指定索引的属性标志（仅修改当前对象）
	 * @param index 属性索引
	 * @param flags 新的属性标志
	 */
	void SetPropertyFlags(PropertySlotIndex index, uint32_t flags) {
		if (index >= 0 && index < static_cast<PropertySlotIndex>(properties_.size())) {
			properties_[index].flags = flags;
		}
	}

protected:
	/**
	 * @brief 属性存储结构（值 + 标志）
	 *
	 * 每个对象都有独立的属性存储，避免 Shape 共享导致的标志冲突问题
	 */
	struct PropertySlot {
		Value value;           ///< 属性值
		uint32_t flags;        ///< 属性标志（每个对象独立）

		PropertySlot() : flags(ShapeProperty::kDefault) {}
		PropertySlot(Value&& v) : value(std::move(v)), flags(ShapeProperty::kDefault) {}
		PropertySlot(Value&& v, uint32_t f) : value(std::move(v)), flags(f) {}
	};

	/**
	 * @brief 获取属性值引用
	 */
	Value& GetPropertyValue(PropertySlotIndex index) {
		return properties_[index].value;
	}

	/**
	 * @brief 获取属性值引用
	 */
	const Value& GetPropertyValue(PropertySlotIndex index) const {
		return properties_[index].value;
	}

	/**
	 * @brief 设置属性值
	 */
	void SetPropertyValue(PropertySlotIndex index, Value&& value) {
		properties_[index].value = std::move(value);
	}

	/**
	 * @brief 添加新属性槽
	 */
	void AddPropertySlot(PropertySlotIndex index, Value&& value, uint32_t flags) {
		if (index < static_cast<PropertySlotIndex>(properties_.size())) {
			properties_[index] = PropertySlot(std::move(value), flags);
		} else {
			assert(index == static_cast<PropertySlotIndex>(properties_.size()));
			properties_.emplace_back(std::move(value), flags);
		}
	}

protected:
	union {
		uint64_t full_ = 0;                   ///< 完整64位值
		struct {
			uint32_t ref_count_;                ///< 引用计数
			static_assert(sizeof(uint16_t) == sizeof(ClassId));
			uint32_t class_id_ : 16;			///< 类id
			uint32_t gc_mark_ : 1;              ///< 垃圾回收标记位
			uint32_t is_extensible_ : 1;        ///< 是否可扩展（JS 标准）
			uint32_t is_frozen_ : 1;            ///< 是否已冻结（JS 标准）
			uint32_t is_sealed_ : 1;            ///< 是否已密封（JS 标准）
			uint32_t set_proto_ : 1;			///< 是否设置了__proto__
			uint32_t reserved_ : 27;            ///< 保留位
		};
	} tag_;
	Shape* shape_;                          ///< 形状指针（对象布局描述，包含原型信息）
	std::vector<PropertySlot> properties_;  ///< 属性槽向量（每个对象独立）
};

} // namespace mjs
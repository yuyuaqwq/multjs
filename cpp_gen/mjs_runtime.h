/**
 * @file mjs_runtime.h
 * @brief JavaScript转C++运行时支持库
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <stdexcept>
#include <memory>

#include <mjs/value.h>
#include <mjs/object.h>
#include <mjs/value/object/array_object.h>

namespace mjs {
namespace generated {

/**
 * @class JSValue
 * @brief 动态类型值，用于无法静态推断的场景
 */
class JSValue : public mjs::Value {

};

/**
 * @class JSObject
 * @brief 动态对象基类，提供运行时属性访问支持
 *
 * 生成的结构体继承此类，同时支持编译期静态属性访问和运行期动态属性访问
 * 此类继承自mjs::Object，集成垃圾回收系统
 *
 * @note 生成的结构体会定义静态属性作为成员变量，
 *       动态属性通过mjs::Object的values_管理
 */
class JSObject : public mjs::Object {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~JSObject() = default;

protected:
    /**
     * @brief 受保护构造函数
     * @param context 执行上下文
     */
    JSObject(mjs::Context* context)
        : mjs::Object(context, static_cast<mjs::ClassId>(0)) {}
};

/**
 * @class JSArray
 * @brief 动态数组，用于无法静态推断的数组元素类型
 */
class JSArray : public mjs::ArrayObject {

};

/**
 * @brief 辅助函数：从对象中获取计算属性，返回 mjs::Value
 *
 * 这个函数封装了 GetComputedProperty 的调用，使得可以在表达式中直接使用
 *
 * @param obj 数组或对象
 * @param context 执行上下文
 * @param key 键（会用 mjs::Value 包装）
 * @return 属性值，如果获取失败则返回 undefined
 */
inline mjs::Value GetComputedProperty(mjs::ArrayObject* obj, mjs::Context* context, int64_t key) {
    mjs::Value result;
    obj->GetComputedProperty(context, mjs::Value(key), &result);
    return result;
}

/**
 * @brief 辅助函数：从对象中获取计算属性（字符串键）
 */
inline mjs::Value GetComputedProperty(mjs::Object* obj, mjs::Context* context, const std::string& key) {
    mjs::Value result;
    obj->GetComputedProperty(context, mjs::Value(key.c_str()), &result);
    return result;
}

} // namespace generated
} // namespace mjs

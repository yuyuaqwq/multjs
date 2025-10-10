/**
 * @file noncopyable.h
 * @brief 不可拷贝基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了不可拷贝基类，用于防止类的拷贝构造和拷贝赋值操作，
 * 确保类的单例特性或资源独占性。
 */

#pragma once

namespace mjs {

/**
 * @class noncopyable
 * @brief 不可拷贝基类
 *
 * 提供不可拷贝功能，防止派生类被意外拷贝。适用于需要确保单例特性
 * 或资源独占性的类，如垃圾回收管理器、运行时环境等。
 *
 * @note 继承此类即可自动禁用拷贝构造和拷贝赋值操作
 * @warning 派生类需要显式定义移动构造和移动赋值操作
 */
class noncopyable {
public:
    /** @brief 删除拷贝构造函数 */
    noncopyable(const noncopyable&) = delete;

    /** @brief 删除拷贝赋值运算符 */
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    /** @brief 默认构造函数 */
    noncopyable() = default;

    /** @brief 默认析构函数 */
    ~noncopyable() = default;
};

} // namespace mjs
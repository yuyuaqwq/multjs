/**
 * @file regexp_object_class_def.h
 * @brief 正则表达式对象类定义
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <mjs/class_def/class_def.h>

namespace mjs {

/**
 * @class RegExpObjectClassDef
 * @brief 正则表达式对象类定义
 */
class RegExpObjectClassDef : public ClassDef {
public:
    /**
     * @brief 构造函数
     */
    RegExpObjectClassDef(Runtime* runtime);

    /**
     * @brief 构造函数调用处理
     */
    Value NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const override;

    /**
     * @brief RegExp构造函数
     * @param context 执行上下文
     * @param par_count 参数数量
     * @param stack 栈帧
     * @return 新创建的RegExp对象
     */
    static Value RegExpConstructor(Context* context, uint32_t par_count, const StackFrame& stack);

    /**
     * @brief test方法：测试字符串是否匹配
     * @param context 执行上下文
     * @param par_count 参数数量
     * @param stack 栈帧
     * @return 匹配返回true，否则返回false
     */
    static Value Test(Context* context, uint32_t par_count, const StackFrame& stack);

    /**
     * @brief exec方法：执行匹配
     * @param context 执行上下文
     * @param par_count 参数数量
     * @param stack 栈帧
     * @return 匹配结果数组或null
     */
    static Value Exec(Context* context, uint32_t par_count, const StackFrame& stack);

    /**
     * @brief toString方法：转换为字符串
     * @param context 执行上下文
     * @param par_count 参数数量
     * @param stack 栈帧
     * @return /pattern/flags 格式的字符串
     */
    static Value ToString(Context* context, uint32_t par_count, const StackFrame& stack);
};

} // namespace mjs

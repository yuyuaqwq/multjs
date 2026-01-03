/**
 * @file name_mangler.h
 * @brief 名称修饰器，处理JavaScript标识符到C++标识符的转换
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <unordered_set>

namespace mjs {
namespace compiler {
namespace cpp_gen {

/**
 * @class NameMangler
 * @brief 名称修饰器，处理C++关键字冲突和非法标识符
 */
class NameMangler {
public:
    /**
     * @brief 构造函数
     */
    NameMangler();

    /**
     * @brief 修饰标识符名称
     * @param name JavaScript标识符
     * @return C++安全标识符
     */
    std::string Mangle(const std::string& name);

    /**
     * @brief 检查标识符是否需要修饰
     * @param name 标识符
     * @return 是否需要修饰
     */
    bool NeedsMangling(const std::string& name) const;

    /**
     * @brief 添加保留字（自定义不应使用的标识符）
     * @param reserved 保留字
     */
    void AddReservedWord(const std::string& reserved);

private:
    /**
     * @brief 检查是否为C++关键字
     */
    bool IsCppKeyword(const std::string& name) const;

    /**
     * @brief 检查标识符是否合法
     * @param name 标识符
     * @return 是否合法
     */
    bool IsValidIdentifier(const std::string& name) const;

    std::unordered_set<std::string> reserved_words_;
};

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

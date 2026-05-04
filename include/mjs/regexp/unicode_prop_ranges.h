/**
 * @file unicode_prop_ranges.h
 * @brief Unicode属性范围定义用于正则表达式
 *
 * 本文件包含解析器和NFA构建器使用的共享Unicode属性范围数据。
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>
#include <utility>

namespace mjs {

/**
 * @brief 获取给定Unicode属性的字符范围
 *
 * 此函数为常见Unicode属性提供ASCII范围的字符。
 * 完整的Unicode支持需要大量的数据表。
 *
 * @param prop_name 属性名称（如 "L", "Lu", "N", "Zs"）
 * @param ranges 填充(start, end)字符对的输出向量
 * @return 如果属性支持返回true，否则返回false
 *
 * 支持的属性（ASCII子集）：
 * - L / Letter: 所有字母 (a-z, A-Z)
 * - Lu / Uppercase_Letter: 大写字母 (A-Z)
 * - Ll / Lowercase_Letter: 小写字母 (a-z)
 * - N / Number: 数字 (0-9)
 * - Nd / Decimal_Number: 十进制数字 (0-9)
 * - P / Punctuation: ASCII标点符号
 * - Zs / Space_Separator: 空格字符
 * - Z / Separator: 空格、制表符、换行符、回车符
 * - S / Symbol: ASCII符号
 * - Sm / Math_Symbol: 数学符号
 * - C / Other: 控制字符
 * - Cc / Control: 控制字符
 *
 * @note 这是一个简化的实现，仅用于ASCII字符范围。
 *       完整的Unicode支持需要大量的数据表。
 */
bool GetUnicodePropRanges(const std::string& prop_name,
                         std::vector<std::pair<char, char>>& ranges);

} // namespace mjs

/**
 * @file unicode_prop_ranges.cpp
 * @brief Unicode属性范围实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <mjs/regexp/unicode_prop_ranges.h>

namespace mjs {

bool GetUnicodePropRanges(const std::string& prop_name,
                         std::vector<std::pair<char, char>>& ranges) {
    ranges.clear();

    // 简化的Unicode属性支持（仅ASCII范围）
    // 完整支持需要大量的数据表

    if (prop_name == "L" || prop_name == "Letter") {
        // 所有字母
        ranges.push_back({'a', 'z'});
        ranges.push_back({'A', 'Z'});
        return true;
    } else if (prop_name == "Lu" || prop_name == "Uppercase_Letter") {
        // 大写字母
        ranges.push_back({'A', 'Z'});
        return true;
    } else if (prop_name == "Ll" || prop_name == "Lowercase_Letter") {
        // 小写字母
        ranges.push_back({'a', 'z'});
        return true;
    } else if (prop_name == "N" || prop_name == "Number") {
        // 所有数字
        ranges.push_back({'0', '9'});
        return true;
    } else if (prop_name == "Nd" || prop_name == "Decimal_Number") {
        // 十进制数字
        ranges.push_back({'0', '9'});
        return true;
    } else if (prop_name == "P" || prop_name == "Punctuation") {
        // ASCII标点符号
        ranges.push_back({'!', '!'});
        ranges.push_back({'"', '"'});
        ranges.push_back({'#', '#'});
        ranges.push_back({'$', '$'});
        ranges.push_back({'%', '%'});
        ranges.push_back({'&', '&'});
        ranges.push_back({'\'', '\''});
        ranges.push_back({'(', '('});
        ranges.push_back({')', ')'});
        ranges.push_back({'*', '*'});
        ranges.push_back({'+', '+'});
        ranges.push_back({',', ','});
        ranges.push_back({'-', '-'});
        ranges.push_back({'.', '.'});
        ranges.push_back({'/', '/'});
        ranges.push_back({':', ':'});
        ranges.push_back({';', ';'});
        ranges.push_back({'<', '<'});
        ranges.push_back({'=', '='});
        ranges.push_back({'>', '>'});
        ranges.push_back({'?', '?'});
        ranges.push_back({'@', '@'});
        ranges.push_back({'[', '['});
        ranges.push_back({'\\', '\\'});
        ranges.push_back({']', ']'});
        ranges.push_back({'^', '^'});
        ranges.push_back({'_', '_'});
        ranges.push_back({'`', '`'});
        ranges.push_back({'{', '{'});
        ranges.push_back({'|', '|'});
        ranges.push_back({'}', '}'});
        ranges.push_back({'~', '~'});
        return true;
    } else if (prop_name == "Zs" || prop_name == "Space_Separator") {
        // 空格
        ranges.push_back({' ', ' '});
        return true;
    } else if (prop_name == "Z" || prop_name == "Separator") {
        // 分隔符
        ranges.push_back({' ', ' '});
        ranges.push_back({'\t', '\t'});
        ranges.push_back({'\n', '\n'});
        ranges.push_back({'\r', '\r'});
        return true;
    } else if (prop_name == "S" || prop_name == "Symbol") {
        // 符号
        ranges.push_back({'$', '$'});
        ranges.push_back({'+', '+'});
        ranges.push_back({'<', '<'});
        ranges.push_back({'=', '='});
        ranges.push_back({'>', '>'});
        ranges.push_back({'^', '^'});
        ranges.push_back({'`', '`'});
        ranges.push_back({'|', '|'});
        ranges.push_back({'~', '~'});
        return true;
    } else if (prop_name == "Sm" || prop_name == "Math_Symbol") {
        // 数学符号
        ranges.push_back({'+', '+'});
        ranges.push_back({'<', '<'});
        ranges.push_back({'=', '='});
        ranges.push_back({'>', '>'});
        ranges.push_back({'|', '|'});
        ranges.push_back({'~', '~'});
        return true;
    } else if (prop_name == "C" || prop_name == "Other") {
        // 其他（控制字符）
        ranges.push_back({0, 31});
        ranges.push_back({127, 159});
        return true;
    } else if (prop_name == "Cc" || prop_name == "Control") {
        // 控制字符
        ranges.push_back({0, 31});
        ranges.push_back({127, 127});
        return true;
    }

    // 不支持的属性
    return false;
}

} // namespace mjs

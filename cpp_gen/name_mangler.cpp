/**
 * @file name_mangler.cpp
 * @brief NameMangler实现
 */

#include <algorithm>
#include <cctype>
#include <regex>

#include "cpp_gen/name_mangler.h"

namespace mjs {
namespace compiler {
namespace cpp_gen {

namespace {

// C++关键字集合
const std::unordered_set<std::string> kCppKeywords = {
    // 基本类型
    "int", "float", "double", "bool", "char", "wchar_t", "char16_t", "char32_t",
    "void", "auto", "signed", "unsigned", "short", "long", "size_t", "int8_t",
    "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",

    // 控制流
    "if", "else", "for", "while", "do", "switch", "case", "break", "continue",
    "goto", "default", "return",

    // 类型相关
    "class", "struct", "union", "enum", "typename", "typedef", "template",
    "namespace", "using", "typeid", "decltype", "typeof",

    // 访问控制
    "public", "protected", "private", "friend", "virtual", "static",
    "extern", "register", "mutable", "inline", "constexpr", "volatile",
    "const", "consteval", "constinit", "explicit",

    // 内存管理
    "new", "delete", "this", "nullptr",

    // 字面量
    "true", "false", "nullptr",

    // 异常
    "throw", "try", "catch", "noexcept",

    // 运算符关键字
    "and", "or", "not", "xor", "bitand", "bitor", "compl",
    "and_eq", "or_eq", "xor_eq", "not_eq",

    // 面向对象
    "operator", "co_await", "co_return", "co_yield",

    // C++20概念
    "concept", "requires",

    // 其他
    "asm", "fortran", "export", "thread_local", "static_assert", "alignas",
    "alignof", "override", "final"
};

} // anonymous namespace

NameMangler::NameMangler() {
    // 添加常见的STL类型为保留字
    reserved_words_.insert("std");
    reserved_words_.insert("string");
    reserved_words_.insert("vector");
    reserved_words_.insert("map");
    reserved_words_.insert("unordered_map");
    reserved_words_.insert("set");
    reserved_words_.insert("optional");
    reserved_words_.insert("variant");
}

std::string NameMangler::Mangle(const std::string& name) {
    if (!NeedsMangling(name)) {
        return name;
    }

    // 如果是C++关键字，添加 js_ 前缀和 _ 后缀
    if (IsCppKeyword(name)) {
        return "js_" + name + "_";
    }

    // 如果是保留字，添加 js_ 前缀
    if (reserved_words_.find(name) != reserved_words_.end()) {
        return "js_" + name;
    }

    // 处理以数字开头的标识符
    if (!name.empty() && std::isdigit(name[0])) {
        return "_js_" + name;
    }

    // 处理包含特殊字符的标识符
    std::string mangled = name;
    for (size_t i = 0; i < mangled.length(); ++i) {
        if (!std::isalnum(mangled[i]) && mangled[i] != '_') {
            mangled[i] = '_';
        }
    }

    return mangled;
}

bool NameMangler::NeedsMangling(const std::string& name) const {
    return IsCppKeyword(name) ||
           reserved_words_.find(name) != reserved_words_.end() ||
           !IsValidIdentifier(name);
}

void NameMangler::AddReservedWord(const std::string& reserved) {
    reserved_words_.insert(reserved);
}

bool NameMangler::IsCppKeyword(const std::string& name) const {
    return kCppKeywords.find(name) != kCppKeywords.end();
}

bool NameMangler::IsValidIdentifier(const std::string& name) const {
    if (name.empty()) {
        return false;
    }

    // 首字符必须是字母或下划线
    if (!std::isalpha(name[0]) && name[0] != '_') {
        return false;
    }

    // 其余字符必须是字母、数字或下划线
    for (size_t i = 1; i < name.length(); ++i) {
        if (!std::isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }

    return true;
}

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

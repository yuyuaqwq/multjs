/**
 * @file regexp_object.h
 * @brief 正则表达式对象
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 *
 * 本文件定义了 JavaScript 正则表达式对象。
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <mjs/value/object/object.h>
#include <mjs/regexp/regexp_nfa.h>

namespace mjs {

class Context;

/**
 * @class RegExpObject
 * @brief 正则表达式对象
 *
 * 实现JavaScript正则表达式对象，支持：
 * - 模式存储和编译
 * - 标志位支持（g、i、m等）
 * - test() 方法
 * - exec() 方法
 * - toString() 方法
 */
class RegExpObject : public Object {
private:
    /**
     * @brief 构造函数
     */
    RegExpObject(Context* context, const std::string& pattern, const std::string& flags);

public:
    /**
     * @brief 析构函数
     */
    ~RegExpObject() override;

    /**
     * @brief 获取模式字符串
     */
    const std::string& pattern() const { return pattern_; }

    /**
     * @brief 获取标志字符串
     */
    const std::string& flags() const { return flags_; }

    /**
     * @brief 是否启用全局匹配
     */
    bool global() const { return global_; }

    /**
     * @brief 是否忽略大小写
     */
    bool ignore_case() const { return ignore_case_; }

    /**
     * @brief 是否启用多行模式
     */
    bool multiline() const { return multiline_; }

    /**
     * @brief 是否启用dotAll模式
     */
    bool dot_all() const { return dot_all_; }

    /**
     * @brief 是否启用Unicode模式
     */
    bool unicode() const { return unicode_; }

    /**
     * @brief 是否启用sticky模式
     */
    bool sticky() const { return sticky_; }

    /**
     * @brief 是否启用Unicode Sets模式 [/v标志]
     */
    bool unicode_sets() const { return unicode_sets_; }

    /**
     * @brief 是否启用hasIndices模式 [/d标志]
     */
    bool has_indices() const { return has_indices_; }

    /**
     * @brief 获取lastIndex
     */
    uint32_t last_index() const { return last_index_; }

    /**
     * @brief 设置lastIndex
     */
    void set_last_index(uint32_t index) { last_index_ = index; }

    /**
     * @brief 测试字符串是否匹配
     * @param context 执行上下文
     * @param str 要测试的字符串
     * @return 匹配返回true，否则返回false
     */
    bool Test(Context* context, const std::string& str);

    /**
     * @brief 执行匹配
     * @param context 执行上下文
     * @param str 要匹配的字符串
     * @return 匹配结果数组，失败返回nullptr
     */
    Value Exec(Context* context, const std::string& str);

    /**
     * @brief 转换为字符串
     * @return /pattern/flags 格式的字符串
     */
    std::string ToString() const;

    /**
     * @brief 垃圾回收遍历
     */
    void GCTraverse(Context* context, GCTraverseCallback callback) override;

    /**
     * @brief 获取编译后的NFA
     */
    const NFA* GetNFA() const { return nfa_.get(); }

    static std::string ValidateFlags(const std::string& flags);

private:
    /**
     * @brief 编译正则表达式
     */
    bool Compile();

    std::string pattern_;           ///< 模式字符串
    std::string flags_;             ///< 标志字符串

    // 标志位
    bool global_;                   ///< g标志
    bool ignore_case_;              ///< i标志
    bool multiline_;                ///< m标志
    bool dot_all_;                  ///< s标志
    bool unicode_;                  ///< u标志
    bool sticky_;                   ///< y标志
    bool unicode_sets_;             ///< v标志 (Unicode Sets模式)
    bool has_indices_;              ///< d标志 (hasIndices)

    uint32_t last_index_;           ///< 下次匹配的起始位置

    std::unique_ptr<NFA> nfa_;      ///< 编译后的NFA

    std::string compile_error_;
    std::unordered_map<std::string, uint32_t> capture_group_names_;

    friend class GCManager;
    friend class RegExpObjectClassDef;
};

} // namespace mjs

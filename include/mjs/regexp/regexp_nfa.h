/**
 * @file regexp_nfa.h
 * @brief 正则表达式 NFA（非确定性有限自动机）
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 *
 * 本文件定义了正则表达式的NFA结构和匹配算法。
 */

#pragma once

#include <cstdint>
#include <vector>
#include <set>
#include <unordered_set>
#include <optional>
#include <string>
#include <mjs/regexp/regexp_parser.h>

namespace mjs {

/**
 * @struct NFAMatchResult
 * @brief NFA匹配结果
 */
struct NFAMatchResult {
    std::string matched_text;      ///< 匹配的文本
    size_t start_pos;              ///< 匹配的起始位置
    size_t end_pos;                ///< 匹配的结束位置
    std::vector<std::string> captures;  ///< 捕获组文本
    std::vector<std::pair<size_t, size_t>> capture_indices;  ///< 捕获组位置信息 [(start, end), ...]

    NFAMatchResult()
        : start_pos(0), end_pos(0) {}
};

/**
 * @brief NFA状态ID类型
 */
using NFAStateId = uint32_t;

constexpr NFAStateId kInvalidNFAStateId = UINT32_MAX;

/**
 * @brief 字符类范围存储限制
 *
 * 这些常量定义了字符类状态中可存储的最大字符范围数量。
 * 每个范围是一对(start, end)，所以实际数组大小是这些值的2倍。
 *
 * - kCharClassRangeLimit: 用于标准字符类 [a-z]
 * - kUnicodePropRangeLimit: 用于Unicode属性转义 \p{...}
 *
 * 选择这些限制是为了：
 * 1. 保持NFAState大小合理（当前约200字节）
 * 2. 支持常见的ASCII字符类
 * 3. 为未来Unicode扩展留出空间
 */
constexpr size_t kCharClassRangeLimit = 32;   // 存储32个范围 = 64个char
constexpr size_t kUnicodePropRangeLimit = 64; // 存储64个范围 = 128个char

/**
 * @enum NFAStateType
 * @brief NFA状态类型
 */
enum class NFAStateType : uint8_t {
    kMatch,           ///< 接受状态（匹配成功）
    kChar,            ///< 字符转移
    kCharClass,       ///< 字符类转移
    kDot,             ///< . 任意字符
    kDigit,           ///< \d 数字
    kNotDigit,        ///< \D 非数字
    kWord,            ///< \w 单词字符
    kNotWord,         ///< \W 非单词字符
    kSpace,           ///< \s 空白字符
    kNotSpace,        ///< \S 非空白字符
    kEpsilon,         ///< 空转移（epsilon）
    kBoundary,        ///< 边界断言 ^ $ \b \B
    kLookahead,       ///< 前瞻断言
    kLookbehind,      ///< 后瞻断言
    kStar,            ///< * 重复（0次或多次）
    kPlus,            ///< + 重复（1次或多次）
    kQuestion,        ///< ? 重复（0次或1次）
    kCaptureStart,    ///< 捕获组开始标记
    kCaptureEnd,      ///< 捕获组结束标记
    kBackref,         ///< 反向引用
    kUnicodeCodePoint, ///< Unicode码点转移 \u{HHHHHH}
    kUnicodeProp,     ///< Unicode属性转移 \p{...}
    kNotUnicodeProp,  ///< Unicode属性转移 \P{...}（否定）
};

/**
 * @enum BoundaryType
 * @brief 边界类型
 */
enum class BoundaryType : uint8_t {
    kStart,           ///< ^ 字符串开头
    kEnd,             ///< $ 字符串结尾
    kWordBoundary,    ///< \b 词边界
    kNotWordBoundary, ///< \B 非词边界
};

/**
 * @struct NFAState
 * @brief NFA状态
 */
struct NFAState {
    NFAStateType type;
    NFAStateId next1;          ///< 转移状态1
    NFAStateId next2;          ///< 转移状态2（用于epsilon转移）

    // 对于epsilon状态，可能需要更多转移，使用vector存储
    std::vector<NFAStateId> epsilon_transitions;  ///< 额外的epsilon转移

    // 字符转移数据
    union {
        char ch;                          ///< 单个字符
        struct {
            bool negated;
            char ranges[kCharClassRangeLimit * 2];  ///< 字符类范围（成对的start, end）
            size_t range_count;
        } char_class;
        BoundaryType boundary_type;       ///< 边界类型
        struct {
            bool positive;                ///< true为正向前瞻，false为负向前瞻
            NFAStateId lookahead_start;   ///< 前瞻子NFA的起始状态
            NFAStateId lookahead_accept;
        } lookahead;
        struct {
            bool positive;                ///< true为正向后瞻，false为负向后瞻
            NFAStateId lookbehind_start;  ///< 后瞻子NFA的起始状态
            NFAStateId lookbehind_accept; ///< 后瞻子NFA的接受状态
        } lookbehind;
        struct {
            bool greedy;                  ///< true为贪婪，false为非贪婪
            NFAStateId loop_start;        ///< 循环体起始状态
        } repeat;
        struct {
            uint32_t capture_index;       ///< 捕获组索引（从1开始）
        } capture;
        struct {
            uint32_t ref_index;           ///< 反向引用的捕获组索引（从1开始）
        } backref;
        struct {
            uint32_t code_point;          ///< Unicode码点（用于\u{HHHHHH}）
        } unicode_code_point;
        struct {
            bool negated;                 ///< true for \P{...} (negated), false for \p{...}
            char ranges[kUnicodePropRangeLimit * 2];  ///< Unicode属性范围（成对的start, end）
            size_t range_count;
        } unicode_prop;
    };

    explicit NFAState(NFAStateType t)
        : type(t), next1(kInvalidNFAStateId), next2(kInvalidNFAStateId), ch(0) {}
};

/**
 * @class NFA
 * @brief 非确定性有限自动机
 */
class NFA {
public:
    NFA();

    /**
     * @brief 创建新状态
     */
    NFAStateId CreateState(NFAStateType type);

    /**
     * @brief 设置字符转移
     */
    void SetCharTransition(NFAStateId from, NFAStateId to, char ch);

    /**
     * @brief 设置字符类转移
     */
    void SetCharClassTransition(NFAStateId from, NFAStateId to, bool negated,
                                const std::vector<std::pair<char, char>>& ranges);

    /**
     * @brief 设置任意字符转移
     */
    void SetDotTransition(NFAStateId from, NFAStateId to);

    /**
     * @brief 设置特殊类型转移（\d \w \s等）
     */
    void SetTypeTransition(NFAStateId from, NFAStateId to, NFAStateType type);

    /**
     * @brief 设置epsilon转移
     */
    void SetEpsilonTransition(NFAStateId from, NFAStateId to);

    /**
     * @brief 设置边界转移
     */
    void SetBoundaryTransition(NFAStateId from, NFAStateId to, BoundaryType boundary_type);

    /**
     * @brief 设置前瞻转移
     */
    void SetLookaheadTransition(NFAStateId from, NFAStateId to, bool positive,
                                NFAStateId lookahead_start, NFAStateId lookahead_accept);

    /**
     * @brief 设置后瞻转移
     */
    void SetLookbehindTransition(NFAStateId from, NFAStateId to, bool positive, NFAStateId lookbehind_start, NFAStateId lookbehind_accept);

    /**
     * @brief 设置捕获组开始转移
     */
    void SetCaptureStartTransition(NFAStateId from, NFAStateId to, uint32_t capture_index);

    /**
     * @brief 设置捕获组结束转移
     */
    void SetCaptureEndTransition(NFAStateId from, NFAStateId to, uint32_t capture_index);

    /**
     * @brief 设置反向引用转移
     */
    void SetBackrefTransition(NFAStateId from, NFAStateId to, uint32_t ref_index);

    /**
     * @brief 设置Unicode码点转移
     */
    void SetUnicodeCodePointTransition(NFAStateId from, NFAStateId to, uint32_t code_point);

    /**
     * @brief 设置Unicode属性转移
     */
    void SetUnicodePropTransition(NFAStateId from, NFAStateId to, bool negated,
                                   const std::vector<std::pair<char, char>>& ranges);

    /**
     * @brief 设置捕获组数量
     */
    void SetCaptureCount(uint32_t count) { capture_count_ = count; }

    /**
     * @brief 获取捕获组数量
     */
    uint32_t capture_count() const { return capture_count_; }

    /**
     * @brief 设置标志
     */
    void SetFlags(bool multiline, bool dot_all, bool unicode = false, bool unicode_sets = false, bool ignore_case = false) {
        multiline_ = multiline;
        dot_all_ = dot_all;
        unicode_ = unicode;
        unicode_sets_ = unicode_sets;
        ignore_case_ = ignore_case;
    }

    /**
     * @brief 获取multiline标志
     */
    bool multiline() const { return multiline_; }

    /**
     * @brief 获取dot_all标志
     */
    bool dot_all() const { return dot_all_; }

    /**
     * @brief 获取unicode标志
     */
    bool unicode() const { return unicode_; }

    /**
     * @brief 获取unicode_sets标志
     */
    bool unicode_sets() const { return unicode_sets_; }

    /**
     * @brief 获取ignore_case标志
     */
    bool ignore_case() const { return ignore_case_; }

    /**
     * @brief 设置起始状态
     */
    void SetStartState(NFAStateId id) { start_state_ = id; }

    /**
     * @brief 获取起始状态
     */
    NFAStateId start_state() const { return start_state_; }

    /**
     * @brief 设置接受状态
     */
    void SetAcceptState(NFAStateId id) { accept_state_ = id; }

    /**
     * @brief 获取接受状态
     */
    NFAStateId accept_state() const { return accept_state_; }

    /**
     * @brief 获取状态
     */
    NFAState& GetState(NFAStateId id) { return states_[id]; }
    const NFAState& GetState(NFAStateId id) const { return states_[id]; }

    /**
     * @brief 计算epsilon闭包
     */
    std::set<NFAStateId> EpsilonClosure(const std::set<NFAStateId>& states) const;
    std::set<NFAStateId> EpsilonClosure(NFAStateId state) const;

    /**
     * @brief 计算epsilon闭包（带上下文，用于支持边界和前瞻断言）
     */
    std::set<NFAStateId> EpsilonWithContext(const std::set<NFAStateId>& states, const std::string& str, size_t pos) const;
    std::set<NFAStateId> EpsilonWithContext(NFAStateId state, const std::string& str, size_t pos) const;

    /**
     * @brief 移动操作
     */
    std::set<NFAStateId> Move(const std::set<NFAStateId>& states, char ch) const;

    /**
     * @brief 匹配字符串，返回是否匹配
     */
    bool Match(const std::string& str) const;

    /**
     * @brief 匹配字符串，返回匹配结束位置
     */
    std::optional<size_t> MatchEnd(const std::string& str, size_t start_pos = 0) const;

    /**
     * @brief 匹配字符串，返回详细匹配结果（包括匹配文本和位置）
     */
    std::optional<NFAMatchResult> MatchDetail(const std::string& str, size_t start_pos = 0) const;

    /**
     * @brief 获取状态数量
     */
    size_t StateCount() const { return states_.size(); }

private:
    std::vector<NFAState> states_;
    NFAStateId start_state_;
    NFAStateId accept_state_;
    uint32_t capture_count_;  ///< 捕获组数量
    bool multiline_;          ///< 多行模式标志
    bool dot_all_;            ///< dotAll模式标志
    bool unicode_;            ///< Unicode模式标志
    bool unicode_sets_;       ///< Unicode Sets模式标志 (/v)
    bool ignore_case_;        ///< 忽略大小写标志

    /**
     * @brief 安全获取状态引用（带边界检查）
     */
    NFAState& GetStateUnchecked(NFAStateId id) {
        return states_[id];
    }

    const NFAState& GetStateUnchecked(NFAStateId id) const {
        return states_[id];
    }

    /**
     * @brief 匹配字符串（内部实现）
     * @param str 要匹配的字符串
     * @param start_pos 开始位置
     * @param full_match 是否需要完整匹配（消费所有字符）
     */
    std::optional<size_t> MatchEnd(const std::string& str, size_t start_pos, bool full_match) const;

    /**
     * @brief 从指定位置尝试匹配
     */
    std::optional<size_t> TryMatchFrom(const std::string& str, size_t pos) const;

    /**
     * @brief 从指定位置尝试详细匹配（返回匹配文本和位置）
     */
    std::optional<NFAMatchResult> TryMatchFromDetail(const std::string& str, size_t pos) const;

    /**
     * @brief 从指定位置尝试详细匹配（带捕获组支持）
     */
    std::optional<NFAMatchResult> TryMatchFromDetailWithCaptures(const std::string& str, size_t pos) const;

    /**
     * @brief 检查字符是否匹配状态
     */
    bool MatchChar(const NFAState& state, char ch) const;

    /**
     * @brief 检查字符是否匹配字符类
     */
    bool MatchCharClass(const NFAState& state, char ch) const;

    /**
     * @brief 检查边界断言是否匹配
     * @param state 边界状态
     * @param str 输入字符串
     * @param pos 当前位置
     */
    bool MatchBoundary(const NFAState& state, const std::string& str, size_t pos) const;

    /**
     * @brief 检查前瞻断言是否匹配
     * @param state 前瞻状态
     * @param str 输入字符串
     * @param pos 当前位置
     */
    bool MatchLookahead(const NFAState& state, const std::string& str, size_t pos) const;

    /**
     * @brief 检查后瞻断言是否匹配
     * @param state 后瞻状态
     * @param str 输入字符串
     * @param pos 当前位置
     */
    bool MatchLookbehind(const NFAState& state, const std::string& str, size_t pos) const;

    /**
     * @brief 检查反向引用是否匹配
     * @param state 反向引用状态
     * @param str 输入字符串
     * @param pos 当前位置
     * @param captures 捕获组数据
     */
    bool MatchBackref(const NFAState& state, const std::string& str, size_t pos,
                      const std::vector<std::pair<size_t, size_t>>& captures) const;

    /**
     * @brief 递归计算epsilon闭包
     */
    void EpsilonClosureRecursive(NFAStateId state, std::set<NFAStateId>& result) const;

    /**
     * @brief 递归计算epsilon闭包（带上下文）
     */
    void EpsilonWithContextRecursive(NFAStateId state, std::set<NFAStateId>& result, const std::string& str, size_t pos) const;
};

/**
 * @class NFABuilder
 * @brief NFA构造器（Thompson构造法）
 *
 * 从AST构建NFA
 */
class NFABuilder {
public:
    /**
     * @brief 从AST构建NFA
     */
    static NFA Build(RegExpASTNode* root, bool ignore_case = false, bool multiline = false, bool dot_all = false, bool unicode = false, bool unicode_sets = false);

private:
    /**
     * @brief 统计AST中的捕获组数量
     */
    static uint32_t CountCaptures(RegExpASTNode* node);

    /**
     * @brief NFA片段
     */
    struct NFAPair {
        NFAStateId start;
        NFAStateId accept;
    };

    /**
     * @brief 递归构建NFA
     */
    static NFAPair Build(RegExpASTNode* node, NFA& nfa, bool ignore_case);

    /**
     * @brief 创建字符转移
     */
    static NFAPair BuildChar(NFA& nfa, char ch, bool ignore_case);

    /**
     * @brief 创建点转移
     */
    static NFAPair BuildDot(NFA& nfa);

    /**
     * @brief 创建字符类转移
     */
    static NFAPair BuildCharClass(NFA& nfa, const CharClassNode* node);

    /**
     * @brief 创建转义转移
     */
    static NFAPair BuildEscape(NFA& nfa, const EscapeNode* node);

    /**
     * @brief 创建连接
     */
    static NFAPair BuildConcat(NFA& nfa, const ConcatNode* node, bool ignore_case);

    /**
     * @brief 创建选择
     */
    static NFAPair BuildAlternative(NFA& nfa, const AlternativeNode* node, bool ignore_case);

    /**
     * @brief 创建重复（*）
     */
    static NFAPair BuildStar(NFA& nfa, NFAPair child, bool greedy);

    /**
     * @brief 创建重复（+）
     */
    static NFAPair BuildPlus(NFA& nfa, NFAPair child, bool greedy);

    /**
     * @brief 创建重复（?）
     */
    static NFAPair BuildQuestion(NFA& nfa, NFAPair child, bool greedy);

    /**
     * @brief 创建分组
     */
    static NFAPair BuildGroup(NFA& nfa, const GroupNode* node, bool ignore_case);

    /**
     * @brief 创建边界断言
     */
    static NFAPair BuildBoundary(NFA& nfa, const BoundaryNode* node);

    /**
     * @brief 创建前瞻断言
     */
    static NFAPair BuildLookahead(NFA& nfa, const LookaheadNode* node, bool ignore_case);

    /**
     * @brief 创建后瞻断言
     */
    static NFAPair BuildLookbehind(NFA& nfa, const LookbehindNode* node, bool ignore_case);

    /**
     * @brief 创建反向引用
     */
    static NFAPair BuildBackref(NFA& nfa, const EscapeNode* node);

    /**
     * @brief 创建字符类集合操作 [/v标志]
     */
    static NFAPair BuildCharClassSet(NFA& nfa, const CharClassSetNode* node);

    /**
     * @brief 创建字符串字面量 [/v标志]
     */
    static NFAPair BuildStringLiteral(NFA& nfa, const StringLiteralNode* node);

    /**
     * @brief 转换字符大小写
     */
    static char ToLower(char ch);
    static char ToUpper(char ch);
    static bool CharEqualsIgnoreCase(char a, char b);

    /**
     * @brief 获取Unicode属性的字符范围
     * @param prop_name 属性名称（如 "L", "N", "Zs" 等）
     * @param ranges 输出字符范围
     * @return 是否支持该属性
     */
    static bool GetUnicodePropRanges(const std::string& prop_name,
                                      std::vector<std::pair<char, char>>& ranges);
};

} // namespace mjs

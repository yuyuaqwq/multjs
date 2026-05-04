/**
 * @file regexp_parser.h
 * @brief 正则表达式解析器
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 *
 * 本文件定义了正则表达式的抽象语法树（AST）节点和解析器。
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace mjs {

/**
 * @enum RegExpASTNodeType
 * @brief 正则表达式AST节点类型
 */
enum class RegExpASTNodeType {
    kChar,              ///< 单个字符
    kDot,               ///< . 任意字符（除换行符）
    kCharClass,         ///< 字符类 [abc] [^abc]
    kCharClassSet,      ///< 字符类集合操作 [/v标志]
    kStringLiteral,     ///< 字符串字面量 \q{...} [/v标志]
    kConcat,            ///< 连接 ab
    kAlternative,       ///< 选择 a|b
    kStar,              ///< 重复 0次或多次 a*
    kPlus,              ///< 重复 1次或多次 a+
    kQuestion,          ///< 重复 0次或1次 a?
    kGroup,             ///< 分组 (...)
    kEscape,            ///< 转义序列 \d \w \s 等
    kBoundary,          ///< 边界断言 ^ $ \b \B
    kLookahead,         ///< 前瞻断言 (?=...) (?!...)
    kLookbehind,        ///< 后瞻断言 (?<=...) (?<!...)
};

/**
 * @class RegExpASTNode
 * @brief 正则表达式AST节点基类
 */
class RegExpASTNode {
public:
    explicit RegExpASTNode(RegExpASTNodeType type) : type_(type) {}
    virtual ~RegExpASTNode() = default;

    RegExpASTNodeType type() const { return type_; }

private:
    RegExpASTNodeType type_;
};

/**
 * @class CharNode
 * @brief 字符节点
 */
class CharNode : public RegExpASTNode {
public:
    CharNode(char ch, bool ignore_case = false)
        : RegExpASTNode(RegExpASTNodeType::kChar), ch_(ch), ignore_case_(ignore_case) {}

    char ch() const { return ch_; }
    bool ignore_case() const { return ignore_case_; }

private:
    char ch_;
    bool ignore_case_;
};

/**
 * @class DotNode
 * @brief 任意字符节点（.）
 */
class DotNode : public RegExpASTNode {
public:
    DotNode() : RegExpASTNode(RegExpASTNodeType::kDot) {}
};

/**
 * @class CharClassNode
 * @brief 字符类节点 [abc] [^abc]
 */
class CharClassNode : public RegExpASTNode {
public:
    struct Range {
        char start;
        char end;

        Range(char s, char e) : start(s), end(e) {}
    };

    CharClassNode(bool negated, std::vector<Range>&& ranges)
        : RegExpASTNode(RegExpASTNodeType::kCharClass),
          negated_(negated), ranges_(std::move(ranges)) {}

    bool negated() const { return negated_; }
    const std::vector<Range>& ranges() const { return ranges_; }

private:
    bool negated_;
    std::vector<Range> ranges_;
};

/**
 * @class CharClassSetNode
 * @brief 字符类集合操作节点 [/v标志]
 *
 * 支持 ECMAScript 2024 的 Unicode Sets 模式集合操作：
 * - 交集：&&
 * - 差集：--
 * - 并集：|| (隐式支持)
 */
class CharClassSetNode : public RegExpASTNode {
public:
    enum class SetOpType {
        kIntersection,  // && 交集
        kSubtraction,   // -- 差集
        kUnion          // || 并集（可选）
    };

    struct SetOperand {
        std::vector<CharClassNode::Range> ranges;  // 字符范围
        bool negated;                               // 是否否定
        std::string prop_name;                      // Unicode属性名称（如果有）

        SetOperand() : negated(false) {}
        SetOperand(const std::vector<CharClassNode::Range>& r, bool n = false)
            : ranges(r), negated(n) {}
    };

    CharClassSetNode(SetOpType op, std::vector<SetOperand>&& operands)
        : RegExpASTNode(RegExpASTNodeType::kCharClassSet),
          op_type_(op), operands_(std::move(operands)) {}

    SetOpType op_type() const { return op_type_; }
    const std::vector<SetOperand>& operands() const { return operands_; }

private:
    SetOpType op_type_;
    std::vector<SetOperand> operands_;
};

/**
 * @class StringLiteralNode
 * @brief 字符串字面量节点 \q{...} [/v标志]
 *
 * ECMAScript 2024 的 Unicode Sets 模式字符串字面量语法
 * 用于匹配多个字符串中的任意一个
 */
class StringLiteralNode : public RegExpASTNode {
public:
    explicit StringLiteralNode(std::vector<std::string>&& strings)
        : RegExpASTNode(RegExpASTNodeType::kStringLiteral),
          strings_(std::move(strings)) {}

    const std::vector<std::string>& strings() const { return strings_; }

private:
    std::vector<std::string> strings_;
};

/**
 * @class ConcatNode
 * @brief 连接节点
 */
class ConcatNode : public RegExpASTNode {
public:
    explicit ConcatNode(std::vector<std::unique_ptr<RegExpASTNode>>&& children)
        : RegExpASTNode(RegExpASTNodeType::kConcat),
          children_(std::move(children)) {}

    std::vector<std::unique_ptr<RegExpASTNode>>& children() { return children_; }
    const std::vector<std::unique_ptr<RegExpASTNode>>& children() const { return children_; }

private:
    std::vector<std::unique_ptr<RegExpASTNode>> children_;
};

/**
 * @class AlternativeNode
 * @brief 选择节点 a|b
 */
class AlternativeNode : public RegExpASTNode {
public:
    explicit AlternativeNode(std::vector<std::unique_ptr<RegExpASTNode>>&& alternatives)
        : RegExpASTNode(RegExpASTNodeType::kAlternative),
          alternatives_(std::move(alternatives)) {}

    std::vector<std::unique_ptr<RegExpASTNode>>& alternatives() { return alternatives_; }
    const std::vector<std::unique_ptr<RegExpASTNode>>& alternatives() const { return alternatives_; }

private:
    std::vector<std::unique_ptr<RegExpASTNode>> alternatives_;
};

/**
 * @class StarNode
 * @brief 0次或多次重复节点 a*
 */
class StarNode : public RegExpASTNode {
public:
    explicit StarNode(std::unique_ptr<RegExpASTNode>&& child, bool greedy = true)
        : RegExpASTNode(RegExpASTNodeType::kStar),
          child_(std::move(child)), greedy_(greedy) {}

    RegExpASTNode* child() const { return child_.get(); }
    bool greedy() const { return greedy_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    bool greedy_;
};

/**
 * @class PlusNode
 * @brief 1次或多次重复节点 a+
 */
class PlusNode : public RegExpASTNode {
public:
    explicit PlusNode(std::unique_ptr<RegExpASTNode>&& child, bool greedy = true)
        : RegExpASTNode(RegExpASTNodeType::kPlus),
          child_(std::move(child)), greedy_(greedy) {}

    RegExpASTNode* child() const { return child_.get(); }
    bool greedy() const { return greedy_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    bool greedy_;
};

/**
 * @class QuestionNode
 * @brief 0次或1次重复节点 a?
 */
class QuestionNode : public RegExpASTNode {
public:
    explicit QuestionNode(std::unique_ptr<RegExpASTNode>&& child, bool greedy = true)
        : RegExpASTNode(RegExpASTNodeType::kQuestion),
          child_(std::move(child)), greedy_(greedy) {}

    RegExpASTNode* child() const { return child_.get(); }
    bool greedy() const { return greedy_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    bool greedy_;
};

/**
 * @class GroupNode
 * @brief 分组节点 (...)
 */
class GroupNode : public RegExpASTNode {
public:
    explicit GroupNode(std::unique_ptr<RegExpASTNode>&& child, uint32_t index = 0,
                       const std::string& name = "")
        : RegExpASTNode(RegExpASTNodeType::kGroup),
          child_(std::move(child)), capture_index_(index), name_(name) {}

    RegExpASTNode* child() const { return child_.get(); }
    uint32_t capture_index() const { return capture_index_; }
    const std::string& name() const { return name_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    uint32_t capture_index_;
    std::string name_;  // 命名捕获组的名称
};

/**
 * @class EscapeNode
 * @brief 转义序列节点
 */
class EscapeNode : public RegExpASTNode {
public:
    enum class EscapeType {
        kDigit,        ///< \d 数字
        kNotDigit,     ///< \D 非数字
        kWord,         ///< \w 单词字符
        kNotWord,      ///< \W 非单词字符
        kSpace,        ///< \s 空白字符
        kNotSpace,     ///< \S 非空白字符
        kChar,         ///< 普通字符转义
        kBackref,      ///< 反向引用 \1 \2 等
        kHex,          ///< \xHH 十六进制转义
        kUnicode,      ///< \uHHHH Unicode转义（BMP）
        kUnicodeCodePoint, ///< \u{HHHHHH} Unicode码点转义（完整Unicode范围）
        kOctal,        ///< \0, \00, \000 八进制转义
        kNamedBackref, ///< 命名反向引用 \k<name>
        kUnicodeProp,  ///< \p{...} Unicode属性转义
        kNotUnicodeProp, ///< \P{...} Unicode属性转义（否定）
    };

    EscapeNode(EscapeType escape_type, char ch = 0, uint32_t ref_index = 0,
               const std::string& ref_name = "", uint32_t code_point = 0,
               const std::string& prop_name = "")
        : RegExpASTNode(RegExpASTNodeType::kEscape),
          escape_type_(escape_type), ch_(ch), ref_index_(ref_index), ref_name_(ref_name),
          code_point_(code_point), prop_name_(prop_name) {}

    EscapeType escape_type() const { return escape_type_; }
    char ch() const { return ch_; }
    uint32_t ref_index() const { return ref_index_; }
    const std::string& ref_name() const { return ref_name_; }
    uint32_t code_point() const { return code_point_; }
    const std::string& prop_name() const { return prop_name_; }
    void set_escape_type(EscapeType escape_type) { escape_type_ = escape_type; }
    void set_ch(char ch) { ch_ = ch; }
    void set_ref_index(uint32_t ref_index) { ref_index_ = ref_index; }

private:
    EscapeType escape_type_;
    char ch_;
    uint32_t ref_index_;  // 用于数字反向引用
    std::string ref_name_;  // 用于命名反向引用
    uint32_t code_point_;  // 用于 Unicode 码点转义 \u{HHHHHH}
    std::string prop_name_;  // 用于 Unicode 属性转义 \p{...}, \P{...}
};

/**
 * @class BoundaryNode
 * @brief 边界断言节点
 */
class BoundaryNode : public RegExpASTNode {
public:
    enum class BoundaryType {
        kStart,        ///< ^ 字符串开头
        kEnd,          ///< $ 字符串结尾
        kWordBoundary, ///< \b 词边界
        kNotWordBoundary, ///< \B 非词边界
    };

    BoundaryNode(BoundaryType boundary_type)
        : RegExpASTNode(RegExpASTNodeType::kBoundary),
          boundary_type_(boundary_type) {}

    BoundaryType boundary_type() const { return boundary_type_; }

private:
    BoundaryType boundary_type_;
};

/**
 * @class LookaheadNode
 * @brief 前瞻断言节点
 */
class LookaheadNode : public RegExpASTNode {
public:
    LookaheadNode(std::unique_ptr<RegExpASTNode>&& child, bool positive)
        : RegExpASTNode(RegExpASTNodeType::kLookahead),
          child_(std::move(child)), positive_(positive) {}

    RegExpASTNode* child() const { return child_.get(); }
    bool positive() const { return positive_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    bool positive_;  // true为正向前瞻，false为负向前瞻
};

/**
 * @class LookbehindNode
 * @brief 后瞻断言节点
 */
class LookbehindNode : public RegExpASTNode {
public:
    LookbehindNode(std::unique_ptr<RegExpASTNode>&& child, bool positive)
        : RegExpASTNode(RegExpASTNodeType::kLookbehind),
          child_(std::move(child)), positive_(positive) {}

    RegExpASTNode* child() const { return child_.get(); }
    bool positive() const { return positive_; }

private:
    std::unique_ptr<RegExpASTNode> child_;
    bool positive_;  // true为正向后瞻，false为负向后瞻
};

/**
 * @class RegExpParser
 * @brief 正则表达式解析器
 *
 * 将正则表达式字符串解析为抽象语法树（AST）
 */
class RegExpParser {
public:
    explicit RegExpParser(const std::string& pattern, bool unicode_mode = false,
                          bool unicode_sets = false);

    /**
     * @brief 解析正则表达式
     * @return AST根节点，解析失败返回nullptr
     */
    std::unique_ptr<RegExpASTNode> Parse();

    /**
     * @brief 获取错误信息
     */
    const std::string& GetError() const { return error_; }

    const std::unordered_map<std::string, uint32_t>& capture_group_names() const {
        return capture_group_names_;
    }

private:
    /**
     * @brief 解析选择表达式（最低优先级）
     */
    std::unique_ptr<RegExpASTNode> ParseDisjunction();

    /**
     * @brief 解析连接表达式
     */
    std::unique_ptr<RegExpASTNode> ParseAlternative();

    /**
     * @brief 解析项（带量词的原子）
     */
    std::unique_ptr<RegExpASTNode> ParseTerm();

    /**
     * @brief 解析原子
     */
    std::unique_ptr<RegExpASTNode> ParseAtom();

    /**
     * @brief 解析分组
     */
    std::unique_ptr<RegExpASTNode> ParseGroup();

    /**
     * @brief 解析字符类
     */
    std::unique_ptr<RegExpASTNode> ParseCharacterClass();

    /**
     * @brief 解析Unicode Sets模式的字符类 [/v标志]
     */
    std::unique_ptr<RegExpASTNode> ParseCharacterClassSet();

    /**
     * @brief 解析字符串字面量 \q{...} [/v标志]
     */
    std::unique_ptr<RegExpASTNode> ParseStringLiteral();

    /**
     * @brief 解析转义序列
     */
    std::unique_ptr<RegExpASTNode> ParseEscape();

    /**
     * @brief 解析前瞻断言
     */
    std::unique_ptr<RegExpASTNode> ParseLookahead();

    /**
     * @brief 解析后瞻断言
     */
    std::unique_ptr<RegExpASTNode> ParseLookbehind();

    /**
     * @brief 查看当前字符
     */
    char Peek() const;

    /**
     * @brief 获取当前字符并前进
     */
    char Advance();

    /**
     * @brief 检查是否还有更多字符
     */
    bool HasMore() const;

    /**
     * @brief 设置错误信息
     */
    void SetError(const std::string& error);

    /**
     * @brief 克隆AST节点（深拷贝）
     */
    std::unique_ptr<RegExpASTNode> CloneASTNode(RegExpASTNode* node);
    bool ResolveBackreferences(RegExpASTNode* node);
    bool ValidateNamedCaptureGroups(RegExpASTNode* node);
    bool CollectNamedCapturePaths(
        RegExpASTNode* node,
        std::vector<std::unordered_map<std::string, bool>>& paths);

    /**
     * @brief 展开量词为基本的AST节点
     * @param atom 要重复的原子
     * @param n 最小重复次数
     * @param m 最大重复次数（SIZE_MAX表示无上限）
     * @param greedy 是否贪婪匹配
     */
    std::unique_ptr<RegExpASTNode> ExpandQuantifier(
        std::unique_ptr<RegExpASTNode> atom, size_t n, size_t m, bool greedy);

    std::string pattern_;
    size_t pos_;
    std::string error_;
    uint32_t capture_count_;
    bool unicode_mode_;
    bool unicode_sets_;  // /v 标志：Unicode Sets模式
    std::unordered_map<std::string, uint32_t> capture_group_names_;  // 命名捕获组名称到索引的映射
};

} // namespace mjs

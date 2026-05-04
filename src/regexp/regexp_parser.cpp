/**
 * @file regexp_parser.cpp
 * @brief 正则表达式解析器实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <mjs/regexp/regexp_parser.h>
#include <mjs/regexp/unicode_prop_ranges.h>
#include <cctype>
#include <format>

namespace mjs {

namespace {

bool IsIdentifierStart(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
           (ch >= 'A' && ch <= 'Z') ||
           ch == '_' || ch == '$';
}

bool IsIdentifierContinue(char ch) {
    return IsIdentifierStart(ch) ||
           (ch >= '0' && ch <= '9');
}

}  // namespace

RegExpParser::RegExpParser(const std::string& pattern, bool unicode_mode, bool unicode_sets)
    : pattern_(pattern),
      pos_(0),
      capture_count_(0),
      unicode_mode_(unicode_mode || unicode_sets),
      unicode_sets_(unicode_sets) {}

std::unique_ptr<RegExpASTNode> RegExpParser::Parse() {
    pos_ = 0;
    error_.clear();
    capture_count_ = 0;

    if (pattern_.empty()) {
        // 空正则表达式匹配空字符串
        return std::make_unique<ConcatNode>(std::vector<std::unique_ptr<RegExpASTNode>>());
    }

    auto root = ParseDisjunction();

    if (!root) {
        return nullptr;
    }

    if (HasMore()) {
        SetError(std::format("Unexpected character at position {}", pos_));
        return nullptr;
    }

    if (!ValidateNamedCaptureGroups(root.get()) || !ResolveBackreferences(root.get())) {
        return nullptr;
    }

    return root;
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseDisjunction() {
    // disjunction :: alternative (| alternative)*
    std::vector<std::unique_ptr<RegExpASTNode>> alternatives;

    auto first = ParseAlternative();
    if (!first) {
        return nullptr;
    }
    alternatives.push_back(std::move(first));

    while (HasMore() && Peek() == '|') {
        Advance();  // 消耗 '|'

        auto next = ParseAlternative();
        if (!next) {
            return nullptr;
        }
        alternatives.push_back(std::move(next));
    }

    if (alternatives.size() == 1) {
        return std::move(alternatives[0]);
    }

    return std::make_unique<AlternativeNode>(std::move(alternatives));
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseAlternative() {
    // alternative :: term*
    std::vector<std::unique_ptr<RegExpASTNode>> terms;

    while (HasMore()) {
        char ch = Peek();

        // 结束条件
        if (ch == '|' || ch == ')') {
            break;
        }

        auto term = ParseTerm();
        if (!term) {
            return nullptr;
        }
        terms.push_back(std::move(term));
    }

    // 总是返回 ConcatNode，即使只有一个子项
    // 这确保了 AST 结构的一致性，并符合测试期望
    return std::make_unique<ConcatNode>(std::move(terms));
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseTerm() {
    // term :: atom (quantifier)?
    auto atom = ParseAtom();
    if (!atom) {
        return nullptr;
    }

    if (!HasMore()) {
        return atom;
    }

    char ch = Peek();
    bool greedy = true;

    // 检查量词
    switch (ch) {
        case '*': {
            Advance();
            // 检查是否为非贪婪量词
            if (HasMore() && Peek() == '?') {
                greedy = false;
                Advance();
            }
            return std::make_unique<StarNode>(std::move(atom), greedy);
        }

        case '+': {
            Advance();
            // 检查是否为非贪婪量词
            if (HasMore() && Peek() == '?') {
                greedy = false;
                Advance();
            }
            return std::make_unique<PlusNode>(std::move(atom), greedy);
        }

        case '?': {
            Advance();
            // 检查是否为非贪婪量词
            if (HasMore() && Peek() == '?') {
                greedy = false;
                Advance();
            }
            return std::make_unique<QuestionNode>(std::move(atom), greedy);
        }

        case '{': {
            size_t saved_pos = pos_;
            auto restore_as_literal = [&]() -> std::unique_ptr<RegExpASTNode> {
                pos_ = saved_pos;
                return std::move(atom);
            };
            // 解析 {n}, {n,}, {n,m} 量词
            Advance();  // 消耗 '{'

            // 解析第一个数字（n）
            if (!HasMore() || !std::isdigit(static_cast<unsigned char>(Peek()))) {
                if (!unicode_mode_) {
                    return restore_as_literal();
                }
                SetError("Invalid quantifier, expected digit after '{'");
                return nullptr;
            }

            size_t n = 0;
            size_t max_quantifier = 1000000;  // 设置合理的上限（100万）

            while (HasMore() && std::isdigit(static_cast<unsigned char>(Peek()))) {
                // 检查溢出
                if (n > (max_quantifier - 9) / 10) {
                    if (!unicode_mode_) {
                        return restore_as_literal();
                    }
                    SetError("Quantifier too large");
                    return nullptr;
                }
                n = n * 10 + (Advance() - '0');

                // 检查是否超过上限
                if (n > max_quantifier) {
                    if (!unicode_mode_) {
                        return restore_as_literal();
                    }
                    SetError(std::format("Quantifier exceeds maximum limit of {}", max_quantifier));
                    return nullptr;
                }
            }

            size_t m = n;  // 默认m等于n

            // 检查是否有逗号
            if (HasMore() && Peek() == ',') {
                Advance();  // 消耗 ','

                // 检查是否有第二个数字（m）
                if (HasMore() && std::isdigit(static_cast<unsigned char>(Peek()))) {
                    m = 0;
                    while (HasMore() && std::isdigit(static_cast<unsigned char>(Peek()))) {
                        // 检查溢出
                        if (m > (max_quantifier - 9) / 10) {
                            if (!unicode_mode_) {
                                return restore_as_literal();
                            }
                            SetError("Quantifier too large");
                            return nullptr;
                        }
                        m = m * 10 + (Advance() - '0');

                        // 检查是否超过上限
                        if (m > max_quantifier) {
                            if (!unicode_mode_) {
                                return restore_as_literal();
                            }
                            SetError(std::format("Quantifier exceeds maximum limit of {}", max_quantifier));
                            return nullptr;
                        }
                    }
                } else {
                    // {n,} 表示n次或更多
                    m = SIZE_MAX;  // 无限大
                }
            }

            // 验证量词范围的有效性
            if (m != SIZE_MAX && m < n) {
                if (!unicode_mode_) {
                    return restore_as_literal();
                }
                SetError("Quantifier range invalid (minimum > maximum)");
                return nullptr;
            }

            // 检查是否有'}'
            if (!HasMore() || Peek() != '}') {
                if (!unicode_mode_) {
                    return restore_as_literal();
                }
                SetError("Unterminated quantifier, expected '}'");
                return nullptr;
            }
            Advance();  // 消耗 '}'

            // 检查是否为非贪婪量词
            if (HasMore() && Peek() == '?') {
                greedy = false;
                Advance();
            }

            // 将量词展开为基本的AST节点
            return ExpandQuantifier(std::move(atom), n, m, greedy);
        }

        default:
            return atom;
    }
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseAtom() {
    if (!HasMore()) {
        SetError("Unexpected end of pattern");
        return nullptr;
    }

    char ch = Peek();

    switch (ch) {
        case '.': {
            Advance();
            return std::make_unique<DotNode>();
        }

        case '^': {
            Advance();
            return std::make_unique<BoundaryNode>(BoundaryNode::BoundaryType::kStart);
        }

        case '$': {
            Advance();
            return std::make_unique<BoundaryNode>(BoundaryNode::BoundaryType::kEnd);
        }

        case '(':
            return ParseGroup();

        case '[':
            return ParseCharacterClass();

        case '\\':
            return ParseEscape();

        case '|':
        case ')':
        case '*':
        case '+':
        case '?':
            SetError(std::format("Unexpected character '{}'", ch));
            return nullptr;

        case '{':
        case '}':
            if (unicode_mode_) {
                SetError(std::format("Unexpected character '{}'", ch));
                return nullptr;
            }
            Advance();
            return std::make_unique<CharNode>(ch);

        default: {
            // 普通字符
            Advance();
            return std::make_unique<CharNode>(ch);
        }
    }
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseGroup() {
    Advance();  // 消耗 '('

    if (HasMore() && Peek() == '?') {
        Advance();  // 消耗 '?'

        if (!HasMore()) {
            SetError("Invalid group");
            return nullptr;
        }

        char next = Peek();
        if (next == ':') {
            Advance();  // 消耗 ':'
            // 非捕获组 (?:...)
            auto group = ParseDisjunction();
            if (!group) {
                return nullptr;
            }

            if (!HasMore() || Peek() != ')') {
                SetError("Unterminated group");
                return nullptr;
            }
            Advance();  // 消耗 ')'

            return group;
        } else if (next == '=') {
            Advance();  // 消耗 '='
            // 正向前瞻 (?=...)
            return ParseLookahead();
        } else if (next == '!') {
            Advance();  // 消耗 '!'
            // 负向前瞻 (?!...)
            return ParseLookahead();
        } else if (next == '<') {
            Advance();  // 消耗 '<'

            if (!HasMore()) {
                SetError("Invalid lookbehind or named capture group");
                return nullptr;
            }

            char next_char = Peek();
            if (next_char == '=' || next_char == '!') {
                Advance();  // 消耗 '=' 或 '!'
                // 后瞻断言 (?<=...) 或 (?<!...)
                return ParseLookbehind();
            } else {
                // 命名捕获组 (?<name>...)
                // 读取名称
                std::string name;
                while (HasMore()) {
                    char name_ch = Peek();
                    if (name_ch == '>') {
                        break;
                    }
                    // 名称可以包含字母、数字、下划线
                    if ((name.empty() && IsIdentifierStart(name_ch)) ||
                        (!name.empty() && IsIdentifierContinue(name_ch))) {
                        name += Advance();
                    } else {
                        SetError("Invalid character in capture group name");
                        return nullptr;
                    }
                }

                if (name.empty()) {
                    SetError("Invalid capture group name");
                    return nullptr;
                }

                if (!HasMore() || Peek() != '>') {
                    SetError("Unterminated named capture group, expected '>'");
                    return nullptr;
                }
                Advance();  // 消耗 '>'

                // 解析命名捕获组的内容
                capture_count_++;
                uint32_t index = capture_count_;

                auto group = ParseDisjunction();
                if (!group) {
                    return nullptr;
                }

                if (!HasMore() || Peek() != ')') {
                    SetError("Unterminated named capture group");
                    return nullptr;
                }
                Advance();  // 消耗 ')'

                // 创建带有名称的分组节点
                auto group_node = std::make_unique<GroupNode>(std::move(group), index, name);

                // 记录名称到索引的映射
                capture_group_names_[name] = index;

                return group_node;
            }
        } else {
            SetError("Invalid group syntax");
            return nullptr;
        }
    } else {
        // 捕获组
        capture_count_++;
        uint32_t index = capture_count_;

        auto group = ParseDisjunction();
        if (!group) {
            return nullptr;
        }

        if (!HasMore() || Peek() != ')') {
            SetError("Unterminated group");
            return nullptr;
        }
        Advance();  // 消耗 ')'

        return std::make_unique<GroupNode>(std::move(group), index);
    }
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseCharacterClass() {
    Advance();  // 消耗 '['

    bool negated = false;
    if (HasMore() && Peek() == '^') {
        negated = true;
        Advance();
    }

    std::vector<CharClassNode::Range> ranges;

    while (HasMore()) {
        char ch = Peek();

        if (ch == ']') {
            Advance();  // 消耗 ']'
            return std::make_unique<CharClassNode>(negated, std::move(ranges));
        }

        if (ch == '\\') {
            // 字符类中的转义
            auto escape = ParseEscape();
            if (!escape) {
                return nullptr;
            }

            if (escape->type() == RegExpASTNodeType::kEscape) {
                auto escape_node = static_cast<EscapeNode*>(escape.get());
                if (escape_node->escape_type() == EscapeNode::EscapeType::kChar) {
                    ch = escape_node->ch();
                } else if (escape_node->escape_type() == EscapeNode::EscapeType::kDigit) {
                    // \d 在字符类中展开为 0-9
                    ranges.emplace_back('0', '9');
                    continue;
                } else if (escape_node->escape_type() == EscapeNode::EscapeType::kWord) {
                    // \w 在字符类中展开为 a-zA-Z0-9_
                    ranges.emplace_back('a', 'z');
                    ranges.emplace_back('A', 'Z');
                    ranges.emplace_back('0', '9');
                    ranges.emplace_back('_', '_');
                    continue;
                } else if (escape_node->escape_type() == EscapeNode::EscapeType::kSpace) {
                    // \s 在字符类中展开为空格、\t、\n、\r
                    ranges.emplace_back(' ', ' ');
                    ranges.emplace_back('\t', '\t');
                    ranges.emplace_back('\n', '\n');
                    ranges.emplace_back('\r', '\r');
                    continue;
                } else if (escape_node->escape_type() == EscapeNode::EscapeType::kUnicodeCodePoint) {
                    // \u{HHHHHH} Unicode码点转义
                    // 将码点转换为字符（仅支持ASCII范围）
                    uint32_t code_point = escape_node->code_point();
                    if (code_point < 128) {
                        ch = static_cast<char>(code_point);
                    } else {
                        // 对于非ASCII码点，暂时不支持
                        SetError("Unicode code point outside ASCII range not supported in character class");
                        return nullptr;
                    }
                } else if (escape_node->escape_type() == EscapeNode::EscapeType::kUnicodeProp ||
                           escape_node->escape_type() == EscapeNode::EscapeType::kNotUnicodeProp) {
                    // \p{...} 和 \P{...} Unicode属性转义
                    // 展开为对应的字符范围
                    const auto& prop_name = escape_node->prop_name();
                    std::vector<std::pair<char, char>> prop_ranges;
                    bool supported = mjs::GetUnicodePropRanges(prop_name, prop_ranges);
                    if (!supported) {
                        SetError(std::format("Unsupported Unicode property '{}'", prop_name));
                        return nullptr;
                    }
                    // 将属性范围添加到字符类范围
                    for (const auto& r : prop_ranges) {
                        ranges.emplace_back(r.first, r.second);
                    }
                    continue;
                } else {
                    SetError("Escape sequence not supported in character class");
                    return nullptr;
                }
            } else if (escape->type() == RegExpASTNodeType::kBoundary) {
                // \b 在字符类中是退格符
                ch = '\b';
            } else {
                SetError("Invalid escape in character class");
                return nullptr;
            }
        } else {
            Advance();
        }

        // 检查是否为范围
        if (HasMore() && Peek() == '-') {
            Advance();  // 消耗 '-'

            if (!HasMore()) {
                SetError("Unterminated character range");
                return nullptr;
            }

            char end_ch = Peek();
            if (end_ch == ']') {
                // '-' 是最后一个字符，作为普通字符
                ranges.emplace_back(ch, ch);
                ranges.emplace_back('-', '-');
            } else if (end_ch == '\\') {
                auto escape = ParseEscape();
                if (!escape) {
                    return nullptr;
                }
                if (escape->type() == RegExpASTNodeType::kEscape) {
                    auto escape_node = static_cast<EscapeNode*>(escape.get());
                    if (escape_node->escape_type() == EscapeNode::EscapeType::kChar) {
                        end_ch = escape_node->ch();
                        if (static_cast<unsigned char>(ch) > static_cast<unsigned char>(end_ch)) {
                            SetError("Range out of order in character class");
                            return nullptr;
                        }
                        ranges.emplace_back(ch, end_ch);
                    } else if (escape_node->escape_type() == EscapeNode::EscapeType::kUnicodeCodePoint) {
                        // \u{HHHHHH} Unicode码点转义
                        uint32_t code_point = escape_node->code_point();
                        if (code_point < 128) {
                            end_ch = static_cast<char>(code_point);
                            if (static_cast<unsigned char>(ch) > static_cast<unsigned char>(end_ch)) {
                                SetError("Range out of order in character class");
                                return nullptr;
                            }
                            ranges.emplace_back(ch, end_ch);
                        } else {
                            SetError("Unicode code point outside ASCII range not supported in character class");
                            return nullptr;
                        }
                    } else {
                        SetError("Invalid escape in character range");
                        return nullptr;
                    }
                } else {
                    SetError("Invalid escape in character range");
                    return nullptr;
                }
            } else {
                Advance();
                if (static_cast<unsigned char>(ch) > static_cast<unsigned char>(end_ch)) {
                    SetError("Range out of order in character class");
                    return nullptr;
                }
                ranges.emplace_back(ch, end_ch);
            }
        } else {
            ranges.emplace_back(ch, ch);
        }
    }

    SetError("Unterminated character class");
    return nullptr;
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseEscape() {
    Advance();  // 消耗 '\\'

    if (!HasMore()) {
        SetError("Incomplete escape sequence");
        return nullptr;
    }

    char ch = Advance();

    switch (ch) {
        case 'd':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kDigit);

        case 'D':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kNotDigit);

        case 'w':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kWord);

        case 'W':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kNotWord);

        case 's':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kSpace);

        case 'S':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kNotSpace);

        case 'b':
            return std::make_unique<BoundaryNode>(BoundaryNode::BoundaryType::kWordBoundary);

        case 'B':
            return std::make_unique<BoundaryNode>(BoundaryNode::BoundaryType::kNotWordBoundary);

        case 'n':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, '\n');

        case 'r':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, '\r');

        case 't':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, '\t');

        case '0': {
            // 八进制转义 \0, \00, \000, \01, \07, 等（必须以\0开头）
            int octal_value = 0;
            int digits = 0;

            // 读取最多3位八进制数字
            while (digits < 3 && HasMore()) {
                char next_ch = Peek();
                if (next_ch >= '0' && next_ch <= '7') {
                    octal_value = octal_value * 8 + (next_ch - '0');
                    Advance();
                    digits++;
                } else {
                    break;
                }
            }

            // 限制在0-255范围内
            if (octal_value > 255) {
                octal_value = 255;
            }

            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kOctal,
                                                  static_cast<char>(octal_value));
        }

        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': {
            // \1 到 \7 优先作为反向引用
            // 如果后面跟0-7的数字，则作为八进制转义（\11-\77）
            if (HasMore() && Peek() >= '0' && Peek() <= '7') {
                // 八进制转义 \11-\77
                int octal_value = (ch - '0');
                int digits = 1;

                while (digits < 3 && HasMore()) {
                    char next_ch = Peek();
                    if (next_ch >= '0' && next_ch <= '7') {
                        octal_value = octal_value * 8 + (next_ch - '0');
                        Advance();
                        digits++;
                    } else {
                        break;
                    }
                }

                if (octal_value > 255) {
                    octal_value = 255;
                }

                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kOctal,
                                                      static_cast<char>(octal_value));
            } else {
                // 反向引用
                uint32_t ref_index = ch - '0';
                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kBackref, 0, ref_index);
            }
        }

        case '8': case '9': {
            // 反向引用
            uint32_t ref_index = ch - '0';
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kBackref, 0, ref_index);
        }

        case 'x': {
            // 十六进制转义 \xHH
            // 读取2位十六进制数字
            if (!HasMore()) {
                SetError("Incomplete hex escape sequence \\x");
                return nullptr;
            }

            int hex_value = 0;
            int digits = 0;

            // 读取2位十六进制数字
            while (digits < 2 && HasMore()) {
                char next_ch = Peek();
                int digit_value = 0;

                if (next_ch >= '0' && next_ch <= '9') {
                    digit_value = next_ch - '0';
                } else if (next_ch >= 'a' && next_ch <= 'f') {
                    digit_value = 10 + (next_ch - 'a');
                } else if (next_ch >= 'A' && next_ch <= 'F') {
                    digit_value = 10 + (next_ch - 'A');
                } else {
                    break;
                }

                hex_value = hex_value * 16 + digit_value;
                Advance();
                digits++;
            }

            if (digits != 2) {
                SetError("Invalid hex escape sequence \\x, expected 2 hex digits");
                return nullptr;
            }

            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kHex,
                                                  static_cast<char>(hex_value));
        }

        case 'u': {
            // Unicode转义
            // 支持两种格式：
            // 1. \uHHHH - 4位十六进制（BMP范围 0x0000-0xFFFF）
            // 2. \u{HHHHHH} - 1-6位十六进制（完整Unicode范围）

            if (!HasMore()) {
                SetError("Incomplete Unicode escape sequence \\u");
                return nullptr;
            }

            // 检查是否为 \u{HHHHHH} 格式
            if (Peek() == '{') {
                if (!unicode_mode_) {
                    return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, 'u');
                }
                Advance();  // 消耗 '{'

                // 读取1-6位十六进制数字
                uint32_t code_point = 0;
                int digits = 0;

                while (digits < 6 && HasMore()) {
                    char next_ch = Peek();
                    int digit_value = 0;

                    if (next_ch >= '0' && next_ch <= '9') {
                        digit_value = next_ch - '0';
                    } else if (next_ch >= 'a' && next_ch <= 'f') {
                        digit_value = 10 + (next_ch - 'a');
                    } else if (next_ch >= 'A' && next_ch <= 'F') {
                        digit_value = 10 + (next_ch - 'A');
                    } else if (next_ch == '}') {
                        break;
                    } else {
                        SetError("Invalid character in Unicode code point escape");
                        return nullptr;
                    }

                    code_point = code_point * 16 + digit_value;
                    Advance();
                    digits++;
                }

                if (digits == 0) {
                    SetError("Invalid Unicode code point escape \\u{...}, expected at least 1 hex digit");
                    return nullptr;
                }

                if (!HasMore() || Peek() != '}') {
                    SetError("Unterminated Unicode code point escape, expected '}'");
                    return nullptr;
                }
                Advance();  // 消耗 '}'

                // 检查码点是否有效（0x000000-0x10FFFF）
                if (code_point > 0x10FFFF) {
                    SetError("Unicode code point out of range (max 0x10FFFF)");
                    return nullptr;
                }

                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kUnicodeCodePoint,
                                                      0, 0, "", code_point);
            } else {
                // 传统格式 \uHHHH - 读取4位十六进制数字
                int unicode_value = 0;
                int digits = 0;

                while (digits < 4 && HasMore()) {
                    char next_ch = Peek();
                    int digit_value = 0;

                    if (next_ch >= '0' && next_ch <= '9') {
                        digit_value = next_ch - '0';
                    } else if (next_ch >= 'a' && next_ch <= 'f') {
                        digit_value = 10 + (next_ch - 'a');
                    } else if (next_ch >= 'A' && next_ch <= 'F') {
                        digit_value = 10 + (next_ch - 'A');
                    } else {
                        break;
                    }

                    unicode_value = unicode_value * 16 + digit_value;
                    Advance();
                    digits++;
                }

                if (digits != 4) {
                    SetError("Invalid Unicode escape sequence \\u, expected 4 hex digits");
                    return nullptr;
                }

                // 将Unicode码点转换为UTF-8字符
                // 对于BMP字符（0x0000-0xFFFF），可以作为单个或两个字符
                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kUnicode,
                                                      static_cast<char>(unicode_value & 0xFF), 0, "",
                                                      static_cast<uint32_t>(unicode_value));
            }
        }

        case 'p': {
            if (!unicode_mode_) {
                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, 'p');
            }
            // Unicode 属性转义 \p{...}
            if (!HasMore() || Peek() != '{') {
                SetError("Invalid Unicode property escape, expected '{' after \\p");
                return nullptr;
            }
            Advance();  // 消耗 '{'

            // 读取属性名称
            std::string prop_name;
            while (HasMore()) {
                char prop_ch = Peek();
                if (prop_ch == '}') {
                    break;
                }
                // 属性名可以包含字母、数字、下划线
                if ((prop_ch >= 'a' && prop_ch <= 'z') ||
                    (prop_ch >= 'A' && prop_ch <= 'Z') ||
                    (prop_ch >= '0' && prop_ch <= '9') ||
                    prop_ch == '_') {
                    prop_name += Advance();
                } else {
                    SetError("Invalid character in Unicode property escape");
                    return nullptr;
                }
            }

            if (!HasMore() || Peek() != '}') {
                SetError("Unterminated Unicode property escape, expected '}'");
                return nullptr;
            }
            Advance();  // 消耗 '}'

            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kUnicodeProp,
                                                  0, 0, "", 0, prop_name);
        }

        case 'P': {
            if (!unicode_mode_) {
                return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, 'P');
            }
            // Unicode 属性转义 \P{...}（否定）
            if (!HasMore() || Peek() != '{') {
                SetError("Invalid Unicode property escape, expected '{' after \\P");
                return nullptr;
            }
            Advance();  // 消耗 '{'

            // 读取属性名称
            std::string prop_name;
            while (HasMore()) {
                char prop_ch = Peek();
                if (prop_ch == '}') {
                    break;
                }
                // 属性名可以包含字母、数字、下划线
                if ((prop_ch >= 'a' && prop_ch <= 'z') ||
                    (prop_ch >= 'A' && prop_ch <= 'Z') ||
                    (prop_ch >= '0' && prop_ch <= '9') ||
                    prop_ch == '_') {
                    prop_name += Advance();
                } else {
                    SetError("Invalid character in Unicode property escape");
                    return nullptr;
                }
            }

            if (!HasMore() || Peek() != '}') {
                SetError("Unterminated Unicode property escape, expected '}'");
                return nullptr;
            }
            Advance();  // 消耗 '}'

            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kNotUnicodeProp,
                                                  0, 0, "", 0, prop_name);
        }

        case 'k': {
            // 命名反向引用 \k<name>
            if (!HasMore() || Peek() != '<') {
                SetError("Invalid named backreference, expected '<' after \\k");
                return nullptr;
            }
            Advance();  // 消耗 '<'

            // 读取名称
            std::string name;
            while (HasMore()) {
                char name_ch = Peek();
                if (name_ch == '>') {
                    break;
                }
                // 名称可以包含字母、数字、下划线
                if ((name.empty() && IsIdentifierStart(name_ch)) ||
                    (!name.empty() && IsIdentifierContinue(name_ch))) {
                    name += Advance();
                } else {
                    SetError("Invalid character in named backreference");
                    return nullptr;
                }
            }

            if (name.empty()) {
                SetError("Invalid named backreference, expected name");
                return nullptr;
            }

            if (!HasMore() || Peek() != '>') {
                SetError("Unterminated named backreference, expected '>'");
                return nullptr;
            }
            Advance();  // 消耗 '>'

            // 查找名称对应的索引
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kNamedBackref,
                                                  0, 0, name);
        }

        case '\\':
        case '/':
        case '|':
        case '.':
        case '*':
        case '+':
        case '?':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case '^':
        case '$':
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, ch);

        default: {
            // 其他字符作为普通字符
            return std::make_unique<EscapeNode>(EscapeNode::EscapeType::kChar, ch);
        }
    }
}

char RegExpParser::Peek() const {
    if (pos_ >= pattern_.size()) {
        return '\0';
    }
    return pattern_[pos_];
}

char RegExpParser::Advance() {
    if (pos_ >= pattern_.size()) {
        return '\0';
    }
    return pattern_[pos_++];
}

bool RegExpParser::HasMore() const {
    return pos_ < pattern_.size();
}

void RegExpParser::SetError(const std::string& error) {
    error_ = error;
}

std::unique_ptr<RegExpASTNode> RegExpParser::CloneASTNode(RegExpASTNode* node) {
    if (!node) {
        return nullptr;
    }

    switch (node->type()) {
        case RegExpASTNodeType::kChar: {
            auto* char_node = static_cast<CharNode*>(node);
            return std::make_unique<CharNode>(char_node->ch(), char_node->ignore_case());
        }
        case RegExpASTNodeType::kDot: {
            return std::make_unique<DotNode>();
        }
        case RegExpASTNodeType::kCharClass: {
            auto* class_node = static_cast<CharClassNode*>(node);
            std::vector<CharClassNode::Range> ranges;
            for (const auto& r : class_node->ranges()) {
                ranges.push_back(r);
            }
            return std::make_unique<CharClassNode>(class_node->negated(), std::move(ranges));
        }
        case RegExpASTNodeType::kEscape: {
            auto* escape_node = static_cast<EscapeNode*>(node);
            return std::make_unique<EscapeNode>(escape_node->escape_type(), escape_node->ch(),
                                                  escape_node->ref_index(), escape_node->ref_name(),
                                                  escape_node->code_point(), escape_node->prop_name());
        }
        case RegExpASTNodeType::kGroup: {
            auto* group_node = static_cast<GroupNode*>(node);
            auto child_clone = CloneASTNode(group_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<GroupNode>(std::move(child_clone), group_node->capture_index(), group_node->name());
        }
        case RegExpASTNodeType::kConcat: {
            auto* concat_node = static_cast<ConcatNode*>(node);
            std::vector<std::unique_ptr<RegExpASTNode>> children;
            for (const auto& child : concat_node->children()) {
                auto child_clone = CloneASTNode(child.get());
                if (!child_clone) {
                    return nullptr;
                }
                children.push_back(std::move(child_clone));
            }
            return std::make_unique<ConcatNode>(std::move(children));
        }
        case RegExpASTNodeType::kAlternative: {
            auto* alt_node = static_cast<AlternativeNode*>(node);
            std::vector<std::unique_ptr<RegExpASTNode>> alternatives;
            for (const auto& alt : alt_node->alternatives()) {
                auto alt_clone = CloneASTNode(alt.get());
                if (!alt_clone) {
                    return nullptr;
                }
                alternatives.push_back(std::move(alt_clone));
            }
            return std::make_unique<AlternativeNode>(std::move(alternatives));
        }
        case RegExpASTNodeType::kStar: {
            auto* star_node = static_cast<StarNode*>(node);
            auto child_clone = CloneASTNode(star_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<StarNode>(std::move(child_clone), star_node->greedy());
        }
        case RegExpASTNodeType::kPlus: {
            auto* plus_node = static_cast<PlusNode*>(node);
            auto child_clone = CloneASTNode(plus_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<PlusNode>(std::move(child_clone), plus_node->greedy());
        }
        case RegExpASTNodeType::kQuestion: {
            auto* question_node = static_cast<QuestionNode*>(node);
            auto child_clone = CloneASTNode(question_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<QuestionNode>(std::move(child_clone), question_node->greedy());
        }
        case RegExpASTNodeType::kBoundary: {
            auto* boundary_node = static_cast<BoundaryNode*>(node);
            return std::make_unique<BoundaryNode>(boundary_node->boundary_type());
        }
        case RegExpASTNodeType::kLookahead: {
            auto* lookahead_node = static_cast<LookaheadNode*>(node);
            auto child_clone = CloneASTNode(lookahead_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<LookaheadNode>(std::move(child_clone), lookahead_node->positive());
        }
        case RegExpASTNodeType::kLookbehind: {
            auto* lookbehind_node = static_cast<LookbehindNode*>(node);
            auto child_clone = CloneASTNode(lookbehind_node->child());
            if (!child_clone) {
                return nullptr;
            }
            return std::make_unique<LookbehindNode>(std::move(child_clone), lookbehind_node->positive());
        }
        default: {
            SetError("Cannot clone node for quantifier expansion");
            return nullptr;
        }
    }
}

bool RegExpParser::ResolveBackreferences(RegExpASTNode* node) {
    if (!node) {
        return true;
    }

    switch (node->type()) {
        case RegExpASTNodeType::kEscape: {
            auto* escape = static_cast<EscapeNode*>(node);
            if (escape->escape_type() == EscapeNode::EscapeType::kNamedBackref) {
                auto it = capture_group_names_.find(escape->ref_name());
                if (it == capture_group_names_.end()) {
                    SetError(std::format("Undefined named capture group '{}'", escape->ref_name()));
                    return false;
                }
                escape->set_ref_index(it->second);
            } else if (escape->escape_type() == EscapeNode::EscapeType::kBackref &&
                       escape->ref_index() > capture_count_) {
                if (unicode_mode_) {
                    SetError(std::format("Invalid escape '\\{}' in Unicode mode", escape->ref_index()));
                    return false;
                }

                if (escape->ref_index() <= 7) {
                    escape->set_escape_type(EscapeNode::EscapeType::kOctal);
                    escape->set_ch(static_cast<char>(escape->ref_index()));
                    escape->set_ref_index(0);
                } else {
                    escape->set_escape_type(EscapeNode::EscapeType::kChar);
                    escape->set_ch(static_cast<char>('0' + escape->ref_index()));
                    escape->set_ref_index(0);
                }
            }
            return true;
        }

        case RegExpASTNodeType::kGroup:
            return ResolveBackreferences(static_cast<GroupNode*>(node)->child());

        case RegExpASTNodeType::kConcat: {
            for (const auto& child : static_cast<ConcatNode*>(node)->children()) {
                if (!ResolveBackreferences(child.get())) {
                    return false;
                }
            }
            return true;
        }

        case RegExpASTNodeType::kAlternative: {
            for (const auto& child : static_cast<AlternativeNode*>(node)->alternatives()) {
                if (!ResolveBackreferences(child.get())) {
                    return false;
                }
            }
            return true;
        }

        case RegExpASTNodeType::kStar:
            return ResolveBackreferences(static_cast<StarNode*>(node)->child());
        case RegExpASTNodeType::kPlus:
            return ResolveBackreferences(static_cast<PlusNode*>(node)->child());
        case RegExpASTNodeType::kQuestion:
            return ResolveBackreferences(static_cast<QuestionNode*>(node)->child());
        case RegExpASTNodeType::kLookahead:
            return ResolveBackreferences(static_cast<LookaheadNode*>(node)->child());
        case RegExpASTNodeType::kLookbehind:
            return ResolveBackreferences(static_cast<LookbehindNode*>(node)->child());
        default:
            return true;
    }
}

bool RegExpParser::ValidateNamedCaptureGroups(RegExpASTNode* node) {
    std::vector<std::unordered_map<std::string, bool>> paths;
    return CollectNamedCapturePaths(node, paths);
}

bool RegExpParser::CollectNamedCapturePaths(
    RegExpASTNode* node,
    std::vector<std::unordered_map<std::string, bool>>& paths) {
    if (!node) {
        paths.push_back({});
        return true;
    }

    switch (node->type()) {
        case RegExpASTNodeType::kGroup: {
            auto* group = static_cast<GroupNode*>(node);
            if (!CollectNamedCapturePaths(group->child(), paths)) {
                return false;
            }
            if (group->name().empty()) {
                return true;
            }
            for (auto& path : paths) {
                if (path.contains(group->name())) {
                    SetError(std::format("Duplicate capture group name '{}'", group->name()));
                    return false;
                }
                path.emplace(group->name(), true);
            }
            return true;
        }

        case RegExpASTNodeType::kConcat: {
            std::vector<std::unordered_map<std::string, bool>> combined = {{}};
            for (const auto& child : static_cast<ConcatNode*>(node)->children()) {
                std::vector<std::unordered_map<std::string, bool>> child_paths;
                if (!CollectNamedCapturePaths(child.get(), child_paths)) {
                    return false;
                }

                std::vector<std::unordered_map<std::string, bool>> next_combined;
                for (const auto& prefix : combined) {
                    for (const auto& suffix : child_paths) {
                        auto merged = prefix;
                        for (const auto& [name, _] : suffix) {
                            if (merged.contains(name)) {
                                SetError(std::format("Duplicate capture group name '{}'", name));
                                return false;
                            }
                            merged.emplace(name, true);
                        }
                        next_combined.push_back(std::move(merged));
                    }
                }
                combined = std::move(next_combined);
            }
            paths = std::move(combined);
            return true;
        }

        case RegExpASTNodeType::kAlternative: {
            for (const auto& child : static_cast<AlternativeNode*>(node)->alternatives()) {
                std::vector<std::unordered_map<std::string, bool>> alt_paths;
                if (!CollectNamedCapturePaths(child.get(), alt_paths)) {
                    return false;
                }
                for (auto& path : alt_paths) {
                    paths.push_back(std::move(path));
                }
            }
            return true;
        }

        case RegExpASTNodeType::kStar:
            return CollectNamedCapturePaths(static_cast<StarNode*>(node)->child(), paths);
        case RegExpASTNodeType::kPlus:
            return CollectNamedCapturePaths(static_cast<PlusNode*>(node)->child(), paths);
        case RegExpASTNodeType::kQuestion:
            return CollectNamedCapturePaths(static_cast<QuestionNode*>(node)->child(), paths);
        case RegExpASTNodeType::kLookahead:
            return CollectNamedCapturePaths(static_cast<LookaheadNode*>(node)->child(), paths);
        case RegExpASTNodeType::kLookbehind:
            return CollectNamedCapturePaths(static_cast<LookbehindNode*>(node)->child(), paths);
        default:
            paths.push_back({});
            return true;
    }
}

std::unique_ptr<RegExpASTNode> RegExpParser::ExpandQuantifier(
    std::unique_ptr<RegExpASTNode> atom, size_t n, size_t m, bool greedy) {

    // 设置合理的展开上限，防止内存爆炸
    constexpr size_t MAX_EXPANSION = 1000;

    // 处理 {n} - 精确匹配n次
    if (m == n) {
        if (n > MAX_EXPANSION) {
            SetError(std::format("Quantifier expansion too large ({} exceeds maximum {})", n, MAX_EXPANSION));
            return nullptr;
        }

        std::vector<std::unique_ptr<RegExpASTNode>> children;
        children.reserve(n);  // 预分配空间
        for (size_t i = 0; i < n; ++i) {
            auto cloned = CloneASTNode(atom.get());
            if (!cloned) {
                return nullptr;
            }
            children.push_back(std::move(cloned));
        }
        return std::make_unique<ConcatNode>(std::move(children));
    }

    // 处理 {n,} - 匹配n次或更多
    if (m == SIZE_MAX) {
        if (n > MAX_EXPANSION) {
            SetError(std::format("Quantifier expansion too large ({} exceeds maximum {})", n, MAX_EXPANSION));
            return nullptr;
        }

        // 先匹配n次，然后加上*
        std::vector<std::unique_ptr<RegExpASTNode>> children;
        children.reserve(n + 1);  // 预分配空间
        for (size_t i = 0; i < n; ++i) {
            auto cloned = CloneASTNode(atom.get());
            if (!cloned) {
                return nullptr;
            }
            children.push_back(std::move(cloned));
        }

        // 添加一个*量词的副本
        auto star_atom = CloneASTNode(atom.get());
        if (!star_atom) {
            return nullptr;
        }
        children.push_back(std::make_unique<StarNode>(std::move(star_atom), greedy));

        return std::make_unique<ConcatNode>(std::move(children));
    }

    // 处理 {n,m} - 匹配n到m次
    if (m > n) {
        size_t expansion_size = n + (m - n);
        if (expansion_size > MAX_EXPANSION) {
            SetError(std::format("Quantifier expansion too large ({} exceeds maximum {})", expansion_size, MAX_EXPANSION));
            return nullptr;
        }

        // 先匹配n次，然后添加(m-n)个可选的副本
        std::vector<std::unique_ptr<RegExpASTNode>> children;
        children.reserve(expansion_size);  // 预分配空间
        for (size_t i = 0; i < n; ++i) {
            auto cloned = CloneASTNode(atom.get());
            if (!cloned) {
                return nullptr;
            }
            children.push_back(std::move(cloned));
        }

        // 添加(m-n)个可选的副本，从右到左嵌套
        // 例如 a{2,4} 展开成 aa(a(a)?)
        std::unique_ptr<RegExpASTNode> current = nullptr;
        for (size_t i = 0; i < (m - n); ++i) {
            auto optional_atom = CloneASTNode(atom.get());
            if (!optional_atom) {
                return nullptr;
            }

            if (current == nullptr) {
                // 最内层：a?
                current = std::make_unique<QuestionNode>(std::move(optional_atom), greedy);
            } else {
                // 嵌套：a(current)?
                std::vector<std::unique_ptr<RegExpASTNode>> concat_children;
                concat_children.push_back(std::move(optional_atom));
                concat_children.push_back(std::move(current));
                auto concat = std::make_unique<ConcatNode>(std::move(concat_children));
                current = std::make_unique<QuestionNode>(std::move(concat), greedy);
            }
        }

        if (current != nullptr) {
            children.push_back(std::move(current));
        }

        return std::make_unique<ConcatNode>(std::move(children));
    }

    // 不应该到达这里
    return nullptr;
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseLookahead() {
    // 当前位置在 ?= 或 ?! 之后
    // 需要回退一个字符来判断是正向前瞻还是负向前瞻
    size_t saved_pos = pos_;
    char lookahead_char = pattern_[saved_pos - 1];  // '=' 或 '!'

    bool positive = (lookahead_char == '=');

    auto group = ParseDisjunction();
    if (!group) {
        return nullptr;
    }

    if (!HasMore() || Peek() != ')') {
        SetError("Unterminated lookahead");
        return nullptr;
    }
    Advance();  // 消耗 ')'

    return std::make_unique<LookaheadNode>(std::move(group), positive);
}

std::unique_ptr<RegExpASTNode> RegExpParser::ParseLookbehind() {
    // 当前位置在 ?<= 或 ?<! 之后
    // 需要回退一个字符来判断是正向后瞻还是负向后瞻
    size_t saved_pos = pos_;
    char lookbehind_char = pattern_[saved_pos - 1];  // '=' 或 '!'

    bool positive = (lookbehind_char == '=');

    auto group = ParseDisjunction();
    if (!group) {
        return nullptr;
    }

    if (!HasMore() || Peek() != ')') {
        SetError("Unterminated lookbehind");
        return nullptr;
    }
    Advance();  // 消耗 ')'

    return std::make_unique<LookbehindNode>(std::move(group), positive);
}

} // namespace mjs

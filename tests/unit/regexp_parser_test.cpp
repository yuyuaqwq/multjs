/**
 * @file regexp_parser_test.cpp
 * @brief 正则表达式解析器测试
 *
 * 测试正则表达式解析器的各种功能，包括:
 * - 基本字符匹配
 * - 字符�?[abc], [^abc]
 * - 特殊字符 .
 * - 转义序列 \d, \D, \w, \W, \s, \S
 * - 量词 *, +, ?
 * - 选择 |
 * - 分组 ()
 * - 错误处理
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "mjs/regexp/regexp_parser.h"

namespace mjs {
namespace test {

/**
 * @class RegExpParserTest
 * @brief 正则表达式解析器测试�?
 */
class RegExpParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：解析正则表达式并检查是否成�?
     */
    std::unique_ptr<RegExpASTNode> Parse(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        return parser.Parse();
    }

    /**
     * @brief 辅助方法：解析正则表达式并检查是否失�?
     */
    bool ParseFailed(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        auto result = parser.Parse();
        return result == nullptr;
    }
};

// ============================================================================
// 基本字符匹配测试
// ============================================================================

/**
 * @test 测试单个字符
 */
TEST_F(RegExpParserTest, SingleCharacter) {
    auto root = Parse("a");
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->type(), RegExpASTNodeType::kConcat);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 1);

    auto* char_node = dynamic_cast<CharNode*>(concat->children()[0].get());
    ASSERT_NE(char_node, nullptr);
    EXPECT_EQ(char_node->ch(), 'a');
}

/**
 * @test 测试多个字符连接
 */
TEST_F(RegExpParserTest, CharacterConcatenation) {
    auto root = Parse("abc");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 3);

    auto* char_node1 = dynamic_cast<CharNode*>(concat->children()[0].get());
    ASSERT_NE(char_node1, nullptr);
    EXPECT_EQ(char_node1->ch(), 'a');

    auto* char_node2 = dynamic_cast<CharNode*>(concat->children()[1].get());
    ASSERT_NE(char_node2, nullptr);
    EXPECT_EQ(char_node2->ch(), 'b');

    auto* char_node3 = dynamic_cast<CharNode*>(concat->children()[2].get());
    ASSERT_NE(char_node3, nullptr);
    EXPECT_EQ(char_node3->ch(), 'c');
}

/**
 * @test 测试特殊字符转义
 */
TEST_F(RegExpParserTest, EscapedSpecialCharacters) {
    auto root = Parse("\\.");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kChar);
    EXPECT_EQ(escape_node->ch(), '.');
}

// ============================================================================
// 字符类测�?
// ============================================================================

/**
 * @test 测试基本字符�?
 */
TEST_F(RegExpParserTest, BasicCharacterClass) {
    auto root = Parse("[abc]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 1);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_FALSE(char_class->negated());
    EXPECT_EQ(char_class->ranges().size(), 3);
}

/**
 * @test 测试否定字符�?
 */
TEST_F(RegExpParserTest, NegatedCharacterClass) {
    auto root = Parse("[^abc]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_TRUE(char_class->negated());
}

/**
 * @test 测试字符类范�?
 */
TEST_F(RegExpParserTest, CharacterClassRange) {
    auto root = Parse("[a-z]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_FALSE(char_class->negated());
    EXPECT_EQ(char_class->ranges().size(), 1);

    const auto& range = char_class->ranges()[0];
    EXPECT_EQ(range.start, 'a');
    EXPECT_EQ(range.end, 'z');
}

/**
 * @test 测试数字字符�?
 */
TEST_F(RegExpParserTest, DigitCharacterClass) {
    auto root = Parse("[0-9]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_EQ(char_class->ranges().size(), 1);

    const auto& range = char_class->ranges()[0];
    EXPECT_EQ(range.start, '0');
    EXPECT_EQ(range.end, '9');
}

TEST_F(RegExpParserTest, EmptyCharacterClass) {
    auto root = Parse("[]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    ASSERT_EQ(concat->children().size(), 1u);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_FALSE(char_class->negated());
    EXPECT_TRUE(char_class->ranges().empty());
}

TEST_F(RegExpParserTest, NegatedEmptyCharacterClass) {
    auto root = Parse("[^]");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    ASSERT_EQ(concat->children().size(), 1u);

    auto* char_class = dynamic_cast<CharClassNode*>(concat->children()[0].get());
    ASSERT_NE(char_class, nullptr);
    EXPECT_TRUE(char_class->negated());
    EXPECT_TRUE(char_class->ranges().empty());
}

TEST_F(RegExpParserTest, DescendingCharacterClassRangeFails) {
    EXPECT_TRUE(ParseFailed("[z-a]"));
}

// ============================================================================
// 特殊字符测试
// ============================================================================

/**
 * @test 测试点号（任意字符）
 */
TEST_F(RegExpParserTest, DotCharacter) {
    auto root = Parse(".");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* dot_node = dynamic_cast<DotNode*>(concat->children()[0].get());
    ASSERT_NE(dot_node, nullptr);
}

// ============================================================================
// 转义序列测试
// ============================================================================

/**
 * @test 测试\d转义序列（数字）
 */
TEST_F(RegExpParserTest, DigitEscapeSequence) {
    auto root = Parse("\\d");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kDigit);
}

/**
 * @test 测试\D转义序列（非数字�?
 */
TEST_F(RegExpParserTest, NotDigitEscapeSequence) {
    auto root = Parse("\\D");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kNotDigit);
}

/**
 * @test 测试\w转义序列（单词字符）
 */
TEST_F(RegExpParserTest, WordEscapeSequence) {
    auto root = Parse("\\w");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kWord);
}

/**
 * @test 测试\W转义序列（非单词字符�?
 */
TEST_F(RegExpParserTest, NotWordEscapeSequence) {
    auto root = Parse("\\W");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kNotWord);
}

/**
 * @test 测试\s转义序列（空白字符）
 */
TEST_F(RegExpParserTest, SpaceEscapeSequence) {
    auto root = Parse("\\s");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kSpace);
}

/**
 * @test 测试\S转义序列（非空白字符�?
 */
TEST_F(RegExpParserTest, NotSpaceEscapeSequence) {
    auto root = Parse("\\S");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kNotSpace);
}

// ============================================================================
// 量词测试
// ============================================================================

/**
 * @test 测试*量词�?次或多次�?
 */
TEST_F(RegExpParserTest, StarQuantifier) {
    auto root = Parse("a*");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* star_node = dynamic_cast<StarNode*>(concat->children()[0].get());
    ASSERT_NE(star_node, nullptr);
    EXPECT_TRUE(star_node->greedy());

    auto* char_node = dynamic_cast<CharNode*>(star_node->child());
    ASSERT_NE(char_node, nullptr);
    EXPECT_EQ(char_node->ch(), 'a');
}

/**
 * @test 测试+量词�?次或多次�?
 */
TEST_F(RegExpParserTest, PlusQuantifier) {
    auto root = Parse("a+");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* plus_node = dynamic_cast<PlusNode*>(concat->children()[0].get());
    ASSERT_NE(plus_node, nullptr);
    EXPECT_TRUE(plus_node->greedy());

    auto* char_node = dynamic_cast<CharNode*>(plus_node->child());
    ASSERT_NE(char_node, nullptr);
    EXPECT_EQ(char_node->ch(), 'a');
}

/**
 * @test 测试?量词�?次或1次）
 */
TEST_F(RegExpParserTest, QuestionQuantifier) {
    auto root = Parse("a?");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* question_node = dynamic_cast<QuestionNode*>(concat->children()[0].get());
    ASSERT_NE(question_node, nullptr);
    EXPECT_TRUE(question_node->greedy());

    auto* char_node = dynamic_cast<CharNode*>(question_node->child());
    ASSERT_NE(char_node, nullptr);
    EXPECT_EQ(char_node->ch(), 'a');
}

/**
 * @test 测试非贪婪量�?
 */
TEST_F(RegExpParserTest, NonGreedyQuantifiers) {
    auto root1 = Parse("a*?");
    ASSERT_NE(root1, nullptr);
    auto* star_node = dynamic_cast<StarNode*>(
        dynamic_cast<ConcatNode*>(root1.get())->children()[0].get());
    ASSERT_NE(star_node, nullptr);
    EXPECT_FALSE(star_node->greedy());

    auto root2 = Parse("a+?");
    ASSERT_NE(root2, nullptr);
    auto* plus_node = dynamic_cast<PlusNode*>(
        dynamic_cast<ConcatNode*>(root2.get())->children()[0].get());
    ASSERT_NE(plus_node, nullptr);
    EXPECT_FALSE(plus_node->greedy());

    auto root3 = Parse("a??");
    ASSERT_NE(root3, nullptr);
    auto* question_node = dynamic_cast<QuestionNode*>(
        dynamic_cast<ConcatNode*>(root3.get())->children()[0].get());
    ASSERT_NE(question_node, nullptr);
    EXPECT_FALSE(question_node->greedy());
}

// ============================================================================
// 选择测试
// ============================================================================

/**
 * @test 测试基本选择
 */
TEST_F(RegExpParserTest, BasicAlternation) {
    auto root = Parse("a|b");
    ASSERT_NE(root, nullptr);

    auto* alt = dynamic_cast<AlternativeNode*>(root.get());
    ASSERT_NE(alt, nullptr);
    EXPECT_EQ(alt->alternatives().size(), 2);
}

/**
 * @test 测试多个选择
 */
TEST_F(RegExpParserTest, MultipleAlternation) {
    auto root = Parse("a|b|c");
    ASSERT_NE(root, nullptr);

    auto* alt = dynamic_cast<AlternativeNode*>(root.get());
    ASSERT_NE(alt, nullptr);
    EXPECT_EQ(alt->alternatives().size(), 3);
}

// ============================================================================
// 分组测试
// ============================================================================

/**
 * @test 测试基本分组
 */
TEST_F(RegExpParserTest, BasicGroup) {
    auto root = Parse("(ab)");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 1);

    auto* group = dynamic_cast<GroupNode*>(concat->children()[0].get());
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->capture_index(), 1);
}

/**
 * @test 测试多个分组
 */
TEST_F(RegExpParserTest, MultipleGroups) {
    auto root = Parse("(a)(b)");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 2);

    auto* group1 = dynamic_cast<GroupNode*>(concat->children()[0].get());
    ASSERT_NE(group1, nullptr);
    EXPECT_EQ(group1->capture_index(), 1);

    auto* group2 = dynamic_cast<GroupNode*>(concat->children()[1].get());
    ASSERT_NE(group2, nullptr);
    EXPECT_EQ(group2->capture_index(), 2);
}

// ============================================================================
// 复杂模式测试
// ============================================================================

/**
 * @test 测试邮箱模式
 */
TEST_F(RegExpParserTest, EmailPattern) {
    auto root = Parse("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}");
    ASSERT_NE(root, nullptr);
}

/**
 * @test 测试URL模式
 */
TEST_F(RegExpParserTest, URLPattern) {
    auto root = Parse("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/\\S*)?");
    ASSERT_NE(root, nullptr);
}

/**
 * @test 测试IP地址模式
 */
TEST_F(RegExpParserTest, IPAddressPattern) {
    auto root = Parse("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
    ASSERT_NE(root, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空正则表达式
 */
TEST_F(RegExpParserTest, EmptyPattern) {
    auto root = Parse("");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 0);
}

// ============================================================================
// 错误处理测试
// ============================================================================

/**
 * @test 测试未闭合的分组
 */
TEST_F(RegExpParserTest, UnclosedGroup) {
    EXPECT_TRUE(ParseFailed("(abc"));
}

/**
 * @test 测试未闭合的字符�?
 */
TEST_F(RegExpParserTest, UnclosedCharacterClass) {
    EXPECT_TRUE(ParseFailed("[abc"));
}

/**
 * @test 测试无效的转义序�?
 */
TEST_F(RegExpParserTest, InvalidEscapeSequence) {
    // 目前所有转义序列都被视为字符转义，所以这个测试可能会改变
    auto root = Parse("\\x");
    // ���ܳɹ���ʧ�ܣ�ȡ����ʵ��
}

TEST_F(RegExpParserTest, BareBracesAreLiteralsWithoutUnicodeMode) {
    EXPECT_FALSE(ParseFailed("a{"));
    EXPECT_FALSE(ParseFailed("a}"));
}

TEST_F(RegExpParserTest, BareBracesAreSyntaxErrorsInUnicodeMode) {
    EXPECT_TRUE(ParseFailed("a{", true));
    EXPECT_TRUE(ParseFailed("a}", true));
}

// ============================================================================
// 新增转义序列测试（\xHH, \uHHHH, \0八进制）
// ============================================================================

/**
 * @test 测试十六进制转义序列 \xHH
 */
TEST_F(RegExpParserTest, HexEscapeSequence) {
    auto root = Parse("\\x41");  // 'A'
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kHex);
    EXPECT_EQ(escape_node->ch(), 'A');
}

/**
 * @test 测试十六进制转义序列小写
 */
TEST_F(RegExpParserTest, HexEscapeSequenceLowercase) {
    auto root = Parse("\\x61");  // 'a'
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kHex);
    EXPECT_EQ(escape_node->ch(), 'a');
}

/**
 * @test 测试无效的十六进制转义序�?
 */
TEST_F(RegExpParserTest, InvalidHexEscapeSequence) {
    EXPECT_TRUE(ParseFailed("\\x"));  // 缺少十六进制数字
}

/**
 * @test 测试Unicode转义序列 \uHHHH
 */
TEST_F(RegExpParserTest, UnicodeEscapeSequence) {
    auto root = Parse("\\u0041");  // 'A'
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kUnicode);
}

/**
 * @test 测试无效的Unicode转义序列
 */
TEST_F(RegExpParserTest, InvalidUnicodeEscapeSequence) {
    EXPECT_TRUE(ParseFailed("\\u"));  // 缺少十六进制数字
}

/**
 * @test 测试八进制转义序�?\0
 */
TEST_F(RegExpParserTest, OctalEscapeSequence) {
    auto root = Parse("\\0101");  // 八进�?01 = 十进�?5 = 'A'
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kOctal);
    EXPECT_EQ(escape_node->ch(), 'A');
}

/**
 * @test 测试八进制转义序�?\0 单个数字
 */
TEST_F(RegExpParserTest, OctalEscapeSequenceSingleDigit) {
    auto root = Parse("\\01");  // 八进�?
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);

    auto* escape_node = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape_node, nullptr);
    EXPECT_EQ(escape_node->escape_type(), EscapeNode::EscapeType::kOctal);
    EXPECT_EQ(escape_node->ch(), '\1');
}

// ============================================================================
// 后瞻断言测试
// ============================================================================

/**
 * @test 测试正向后瞻断言
 */
TEST_F(RegExpParserTest, PositiveLookbehind) {
    auto root = Parse("(?<=abc)def");
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_GE(concat->children().size(), 2);  // 至少包含后瞻和后续字�?

    EXPECT_EQ(concat->children()[0]->type(), RegExpASTNodeType::kLookbehind);
    auto* lookbehind = dynamic_cast<LookbehindNode*>(concat->children()[0].get());
    ASSERT_NE(lookbehind, nullptr);
    EXPECT_TRUE(lookbehind->positive());
}

/**
 * @test 测试负向后瞻断言
 */
TEST_F(RegExpParserTest, NegativeLookbehind) {
    auto root = Parse("(?<!abc)def");
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_GE(concat->children().size(), 2);  // 至少包含后瞻和后续字�?

    EXPECT_EQ(concat->children()[0]->type(), RegExpASTNodeType::kLookbehind);
    auto* lookbehind = dynamic_cast<LookbehindNode*>(concat->children()[0].get());
    ASSERT_NE(lookbehind, nullptr);
    EXPECT_FALSE(lookbehind->positive());
}

/**
 * @test 测试未闭合的后瞻断言
 */
TEST_F(RegExpParserTest, UnclosedLookbehind) {
    EXPECT_TRUE(ParseFailed("(?<=abc"));
}

// ============================================================================
// 命名捕获组测�?
// ============================================================================

/**
 * @test 测试命名捕获�?
 */
TEST_F(RegExpParserTest, NamedCaptureGroup) {
    auto root = Parse("(?<name>abc)");
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 1);

    EXPECT_EQ(concat->children()[0]->type(), RegExpASTNodeType::kGroup);
    auto* group = dynamic_cast<GroupNode*>(concat->children()[0].get());
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name(), "name");
    EXPECT_EQ(group->capture_index(), 1);
}

/**
 * @test 测试命名捕获组带数字和下划线
 */
TEST_F(RegExpParserTest, NamedCaptureGroupWithUnderscore) {
    auto root = Parse("(?<my_name123>abc)");
    EXPECT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    EXPECT_EQ(concat->children().size(), 1);

    EXPECT_EQ(concat->children()[0]->type(), RegExpASTNodeType::kGroup);
    auto* group = dynamic_cast<GroupNode*>(concat->children()[0].get());
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name(), "my_name123");
}

/**
 * @test 测试未闭合的命名捕获�?
 */
TEST_F(RegExpParserTest, UnclosedNamedCaptureGroup) {
    EXPECT_TRUE(ParseFailed("(?<name"));
    EXPECT_TRUE(ParseFailed("(?<name>abc"));
}

/**
 * @test 测试命名捕获组无效字�?
 */
TEST_F(RegExpParserTest, NamedCaptureGroupInvalidChar) {
    EXPECT_TRUE(ParseFailed("(?<name-abc>"));  // 无效字符 '-'
}

// ============================================================================
// 命名反向引用测试
// ============================================================================

/**
 * @test 测试命名反向引用
 */
TEST_F(RegExpParserTest, NamedBackreference) {
    auto root = Parse("(?<name>abc)\\k<name>");
    EXPECT_NE(root, nullptr);
    EXPECT_EQ(root->type(), RegExpASTNodeType::kConcat);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 2);

    EXPECT_EQ(concat->children()[0]->type(), RegExpASTNodeType::kGroup);
    auto group = static_cast<GroupNode*>(concat->children()[0].get());
    EXPECT_EQ(group->name(), "name");

    EXPECT_EQ(concat->children()[1]->type(), RegExpASTNodeType::kEscape);
    auto escape = static_cast<EscapeNode*>(concat->children()[1].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kNamedBackref);
    EXPECT_EQ(escape->ref_name(), "name");
}

TEST_F(RegExpParserTest, ForwardNamedBackreference) {
    auto root = Parse("\\k<name>(?<name>abc)");
    EXPECT_NE(root, nullptr);
}

/**
 * @test 测试未定义的命名反向引用
 */
TEST_F(RegExpParserTest, UndefinedNamedBackreference) {
    EXPECT_TRUE(ParseFailed("\\k<undefined>"));  // 未定义的命名捕获�?
}

/**
 * @test 测试未闭合的命名反向引用
 */
TEST_F(RegExpParserTest, UnclosedNamedBackreference) {
    EXPECT_TRUE(ParseFailed("\\k<name"));  // 缺少 '>'
}

// ============================================================================
// Unicode 码点转义测试
// ============================================================================

/**
 * @test 测试 Unicode 码点转义 \u{HHHHHH}
 */
TEST_F(RegExpParserTest, UnicodeCodePointEscape) {
    auto root = Parse("\\u{41}", true);
    EXPECT_NE(root, nullptr);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 1);

    auto escape = static_cast<EscapeNode*>(concat->children()[0].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kUnicodeCodePoint);
    EXPECT_EQ(escape->code_point(), 0x41);  // 'A'
}

/**
 * @test 测试 Unicode 码点转义 \u{HHHHHH}（多位数字）
 */
TEST_F(RegExpParserTest, UnicodeCodePointEscapeMultipleDigits) {
    auto root = Parse("\\u{1F600}", true);
    EXPECT_NE(root, nullptr);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 1);

    auto escape = static_cast<EscapeNode*>(concat->children()[0].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kUnicodeCodePoint);
    EXPECT_EQ(escape->code_point(), 0x1F600);  // 😀
}

/**
 * @test 测试未闭合的 Unicode 码点转义
 */
TEST_F(RegExpParserTest, UnclosedUnicodeCodePointEscape) {
    EXPECT_TRUE(ParseFailed("\\u{41", true));  // ȱ�� '}'
}

/**
 * @test 测试无效�?Unicode 码点（超出范围）
 */
TEST_F(RegExpParserTest, InvalidUnicodeCodePointOutOfRange) {
    EXPECT_TRUE(ParseFailed("\\u{110000}", true));  // �������Χ 0x10FFFF
}

/**
 * @test 测试空的 Unicode 码点转义
 */
TEST_F(RegExpParserTest, EmptyUnicodeCodePointEscape) {
    EXPECT_TRUE(ParseFailed("\\u{}", true));  // ������Ҫ1λʮ����������
}

// ============================================================================
// Unicode 属性转义测�?
// ============================================================================

/**
 * @test 测试 Unicode 属性转�?\p{L}
 */
TEST_F(RegExpParserTest, UnicodePropertyEscapeLetter) {
    auto root = Parse("\\p{L}", true);
    EXPECT_NE(root, nullptr);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 1);

    auto escape = static_cast<EscapeNode*>(concat->children()[0].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kUnicodeProp);
    EXPECT_EQ(escape->prop_name(), "L");
}

/**
 * @test 测试 Unicode 属性转�?\P{L}（否定）
 */
TEST_F(RegExpParserTest, UnicodePropertyEscapeNotLetter) {
    auto root = Parse("\\P{L}", true);
    EXPECT_NE(root, nullptr);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 1);

    auto escape = static_cast<EscapeNode*>(concat->children()[0].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kNotUnicodeProp);
    EXPECT_EQ(escape->prop_name(), "L");
}

/**
 * @test 测试 Unicode 属性转�?\p{Number}
 */
TEST_F(RegExpParserTest, UnicodePropertyEscapeNumber) {
    auto root = Parse("\\p{Number}", true);
    EXPECT_NE(root, nullptr);

    auto concat = static_cast<ConcatNode*>(root.get());
    EXPECT_EQ(concat->children().size(), 1);

    auto escape = static_cast<EscapeNode*>(concat->children()[0].get());
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kUnicodeProp);
    EXPECT_EQ(escape->prop_name(), "Number");
}

/**
 * @test 测试未闭合的 Unicode 属性转�?
 */
TEST_F(RegExpParserTest, UnclosedUnicodePropertyEscape) {
    EXPECT_TRUE(ParseFailed("\\p{L", true));  // ȱ�� '}'
}

/**
 * @test 测试 Unicode 属性转义缺少左花括�?
 */
TEST_F(RegExpParserTest, UnicodePropertyEscapeMissingLeftBrace) {
    EXPECT_TRUE(ParseFailed("\\pL", true));  // ȱ�� '{'
}

TEST_F(RegExpParserTest, UnicodePropertyEscapeRequiresUnicodeMode) {
    auto root = Parse("\\p{L}");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    ASSERT_EQ(concat->children().size(), 4u);

    auto* escape = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape, nullptr);
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kChar);
    EXPECT_EQ(escape->ch(), 'p');

    auto unicode_root = Parse("\\p{L}", true);
    ASSERT_NE(unicode_root, nullptr);
    auto* unicode_concat = dynamic_cast<ConcatNode*>(unicode_root.get());
    ASSERT_NE(unicode_concat, nullptr);
    ASSERT_EQ(unicode_concat->children().size(), 1u);
    auto* unicode_escape = dynamic_cast<EscapeNode*>(unicode_concat->children()[0].get());
    ASSERT_NE(unicode_escape, nullptr);
    EXPECT_EQ(unicode_escape->escape_type(), EscapeNode::EscapeType::kUnicodeProp);
}

TEST_F(RegExpParserTest, NumericEscapeResolvesPerUnicodeMode) {
    auto root = Parse("\\1");
    ASSERT_NE(root, nullptr);

    auto* concat = dynamic_cast<ConcatNode*>(root.get());
    ASSERT_NE(concat, nullptr);
    ASSERT_EQ(concat->children().size(), 1u);
    auto* escape = dynamic_cast<EscapeNode*>(concat->children()[0].get());
    ASSERT_NE(escape, nullptr);
    EXPECT_EQ(escape->escape_type(), EscapeNode::EscapeType::kOctal);
    EXPECT_EQ(escape->ch(), '\1');

    EXPECT_TRUE(ParseFailed("\\1", true));
}

} // namespace test
} // namespace mjs

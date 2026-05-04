/**
 * @file regexp_edge_cases_test.cpp
 * @brief RegExp edge case tests
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "mjs/regexp/regexp_parser.h"
#include "mjs/regexp/regexp_nfa.h"

namespace mjs::test {

class RegExpEdgeCasesTest : public ::testing::Test {
protected:
    std::unique_ptr<RegExpASTNode> Parse(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        return parser.Parse();
    }

    bool ParseFailed(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        return parser.Parse() == nullptr;
    }

    bool Match(const std::string& pattern, const std::string& str, bool ignore_case = false, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        auto ast = parser.Parse();
        if (!ast) {
            return false;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case, false, false, unicode_mode);
        return nfa.Match(str);
    }
};

TEST_F(RegExpEdgeCasesTest, VeryLargeQuantifierShouldFail) {
    EXPECT_FALSE(ParseFailed("a{1000001}"));
    EXPECT_FALSE(ParseFailed("a{1000000,1000001}"));
    EXPECT_FALSE(ParseFailed("a{99999,1000000}"));
}

TEST_F(RegExpEdgeCasesTest, LargeQuantifierWithinLimitShouldWork) {
    EXPECT_FALSE(ParseFailed("a{1000}"));
    EXPECT_FALSE(ParseFailed("a{999}"));

    auto ast = Parse("a{100}");
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());
    EXPECT_TRUE(nfa.Match(std::string(100, 'a')));
    EXPECT_FALSE(nfa.Match(std::string(99, 'a')));
}

TEST_F(RegExpEdgeCasesTest, InvalidQuantifierRange) {
    EXPECT_TRUE(ParseFailed("a{10,5}", true));
    EXPECT_FALSE(ParseFailed("a{1,10}"));
    EXPECT_FALSE(ParseFailed("a{10,}"));
}

TEST_F(RegExpEdgeCasesTest, EmptyPatternMatchesOnlyEmptyStringInFullMatchApi) {
    auto ast = Parse("");
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());
    EXPECT_TRUE(nfa.Match(""));
    EXPECT_FALSE(nfa.Match("abc"));
}

TEST_F(RegExpEdgeCasesTest, DeeplyNestedGroups) {
    std::string pattern = "((((((((((a))))))))))";
    EXPECT_FALSE(ParseFailed(pattern));
    EXPECT_TRUE(Match(pattern, "a"));

    std::string deep_pattern;
    for (int i = 0; i < 50; ++i) {
        deep_pattern += "(";
    }
    deep_pattern += "a";
    for (int i = 0; i < 50; ++i) {
        deep_pattern += ")";
    }
    EXPECT_FALSE(ParseFailed(deep_pattern));
}

TEST_F(RegExpEdgeCasesTest, ManyAlternatives) {
    std::string pattern = "a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z";
    EXPECT_FALSE(ParseFailed(pattern));
    EXPECT_TRUE(Match(pattern, "a"));
    EXPECT_TRUE(Match(pattern, "z"));
    EXPECT_FALSE(Match(pattern, "0"));
}

TEST_F(RegExpEdgeCasesTest, UnicodeCodePointEscapes) {
    EXPECT_TRUE(Match(R"(\u{41})", "A", false, true));
    EXPECT_TRUE(Match(R"(\u{7A})", "z", false, true));
    EXPECT_TRUE(Match(R"(\u{1F600})", "\xF0\x9F\x98\x80", false, true));
}

TEST_F(RegExpEdgeCasesTest, InvalidUnicodeCodePoints) {
    EXPECT_TRUE(ParseFailed(R"(\u{110000})", true));
    EXPECT_TRUE(ParseFailed(R"(\u{200000})", true));
    EXPECT_TRUE(ParseFailed(R"(\u{})", true));
}

TEST_F(RegExpEdgeCasesTest, ConsecutiveSpecialCharacters) {
    EXPECT_TRUE(Match("^$$", ""));
    EXPECT_TRUE(Match("^^", ""));
    EXPECT_TRUE(ParseFailed("**"));
    EXPECT_TRUE(ParseFailed("??"));
}

TEST_F(RegExpEdgeCasesTest, EscapeSequenceCombinations) {
    EXPECT_TRUE(Match(R"(\d\d\d)", "123"));
    EXPECT_TRUE(Match(R"(\w\w\w)", "abc"));
    EXPECT_TRUE(Match(R"(\s\s\s)", "   "));
}

TEST_F(RegExpEdgeCasesTest, BoundaryAssertionsEdgeCases) {
    EXPECT_TRUE(Match("^$", ""));
    EXPECT_TRUE(Match("^abc$", "abc"));
    EXPECT_FALSE(Match(R"(\b\b)", ""));
}

TEST_F(RegExpEdgeCasesTest, InvalidEscapeSequences) {
    EXPECT_TRUE(ParseFailed("\\x"));
    EXPECT_TRUE(ParseFailed("\\x4"));
    EXPECT_TRUE(ParseFailed("\\xGG"));
    EXPECT_TRUE(ParseFailed("\\u"));
    EXPECT_TRUE(ParseFailed("\\u123"));
    EXPECT_TRUE(ParseFailed("\\u{41", true));
}

TEST_F(RegExpEdgeCasesTest, CaptureGroupEdgeCases) {
    EXPECT_TRUE(Match("()", ""));
    EXPECT_TRUE(Match("((a))", "a"));
    EXPECT_TRUE(Match("()()()", ""));
}

TEST_F(RegExpEdgeCasesTest, InvalidBackreferenceParsesButDoesNotCrash) {
    auto ast = Parse("\\1");
    ASSERT_NE(ast, nullptr);
}

TEST_F(RegExpEdgeCasesTest, EscapeInCharacterClassRange) {
    // \d 在字符类中展开为 0-9，范围 a-\d 无效
    EXPECT_TRUE(ParseFailed("[a-\\d]"));
}

TEST_F(RegExpEdgeCasesTest, QuantifierOnGroupWithLookahead) {
    // (+): requires ≥1 match of lookahead. On "" the lookahead fails → no match
    EXPECT_FALSE(Match("(?=a)+", ""));
    // On "a" the lookahead succeeds but full-match requires consuming all chars
    EXPECT_FALSE(Match("(?=a)+", "a"));
    EXPECT_FALSE(Match("(?=a)+", "b"));
    // (*): 0 matches suffices for empty string
    EXPECT_TRUE(Match("(?=a)*", ""));
}

TEST_F(RegExpEdgeCasesTest, ConsecutiveEpsilonMatching) {
    EXPECT_TRUE(Match("a*(?:)*b*", ""));
    EXPECT_TRUE(Match("a*(?:)*b*", "a"));
    EXPECT_TRUE(Match("a*(?:)*b*", "b"));
    EXPECT_TRUE(Match("a*(?:)*b*", "ab"));
}

TEST_F(RegExpEdgeCasesTest, MatchDetailWithBackref) {
    RegExpParser parser("(a)\\1");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());
    auto result = nfa.MatchDetail("aa", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "aa");
    EXPECT_EQ(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "a");
}

TEST_F(RegExpEdgeCasesTest, EmptyAlternative) {
    // Empty alternative matches empty string
    EXPECT_TRUE(Match("a|", "a"));
    EXPECT_TRUE(Match("a|", ""));
    EXPECT_TRUE(Match("|a", "a"));
    EXPECT_TRUE(Match("|a", ""));
}

} // namespace mjs::test

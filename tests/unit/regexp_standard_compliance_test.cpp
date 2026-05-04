/**
 * @file regexp_standard_compliance_test.cpp
 * @brief RegExp standard compliance regression tests
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <mjs/context.h>
#include <mjs/gc/handle.h>
#include <mjs/regexp/regexp_nfa.h>
#include <mjs/regexp/regexp_object.h>
#include <mjs/regexp/regexp_parser.h>
#include <mjs/runtime.h>

#include "tests/unit/test_helpers.h"

namespace mjs::test {

class RegExpStandardComplianceTest : public ::testing::Test {
protected:
    std::unique_ptr<RegExpASTNode> Parse(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        return parser.Parse();
    }

    bool ParseFailed(const std::string& pattern, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        return parser.Parse() == nullptr;
    }

    bool Match(const std::string& pattern, const std::string& input,
               bool ignore_case = false, bool unicode_mode = false) {
        RegExpParser parser(pattern, unicode_mode);
        auto ast = parser.Parse();
        if (!ast) {
            return false;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case, false, false, unicode_mode);
        return nfa.Match(input);
    }

    void SetUp() override {
        test_env_ = std::make_unique<TestEnvironment>();
        context_ = std::make_unique<Context>(test_env_->runtime());
    }

    void TearDown() override {
        context_.reset();
        test_env_.reset();
    }

    std::unique_ptr<TestEnvironment> test_env_;
    std::unique_ptr<Context> context_;
};

TEST_F(RegExpStandardComplianceTest, DuplicateNamedCapturesAcrossAlternativesAreAllowed) {
    EXPECT_FALSE(ParseFailed("(?<value>a)|(?<value>b)"));
}

TEST_F(RegExpStandardComplianceTest, DuplicateNamedCapturesInSamePathAreRejected) {
    EXPECT_TRUE(ParseFailed("(?<value>a)(?<value>b)"));
    EXPECT_TRUE(ParseFailed("(?:(?<value>a)|b)(?<value>c)"));
}

TEST_F(RegExpStandardComplianceTest, ForwardNumericBackreferenceMatchesFutureCapture) {
    EXPECT_TRUE(Match("^\\1(a)$", "a"));
    EXPECT_FALSE(Match("^\\1(a)$", "aa"));
}

TEST_F(RegExpStandardComplianceTest, UnmatchedNamedBackreferenceMatchesEmptyString) {
    EXPECT_TRUE(Match("^(?:(?<value>x)|y)\\k<value>$", "y"));
    EXPECT_TRUE(Match("^(?:(?<value>x)|y)\\k<value>$", "xx"));
    EXPECT_FALSE(Match("^(?:(?<value>x)|y)\\k<value>$", "x"));
}

TEST_F(RegExpStandardComplianceTest, LegacyNumericEscapeEightActsAsLiteral) {
    EXPECT_FALSE(ParseFailed("^\\8$"));
    EXPECT_TRUE(ParseFailed("^\\8$", true));

    EXPECT_TRUE(Match("^\\8$", "8"));
    EXPECT_FALSE(Match("^\\8$", "\b"));
}

TEST_F(RegExpStandardComplianceTest, RegExpObjectSupportsUnmatchedNamedBackreference) {
    GCHandleScope<1> scope(context_.get());
    auto regex = scope.New<RegExpObject>("^(?:(?<value>x)|y)\\k<value>$", "");

    ASSERT_NE(regex->GetNFA(), nullptr);
    EXPECT_TRUE(regex->Test(context_.get(), "y"));
    EXPECT_TRUE(regex->Test(context_.get(), "xx"));
    EXPECT_FALSE(regex->Test(context_.get(), "x"));
}

TEST_F(RegExpStandardComplianceTest, RegExpObjectPreservesLegacyNumericEscapeBehavior) {
    GCHandleScope<2> scope(context_.get());
    auto legacy = scope.New<RegExpObject>("^\\8$", "");
    auto unicode = scope.New<RegExpObject>("^\\8$", "u");

    ASSERT_NE(legacy->GetNFA(), nullptr);
    EXPECT_TRUE(legacy->Test(context_.get(), "8"));
    EXPECT_EQ(unicode->GetNFA(), nullptr);
}

TEST_F(RegExpStandardComplianceTest, EmptyAlternationBranchMatchesEmpty) {
    EXPECT_TRUE(Match("a|", "a"));
    EXPECT_TRUE(Match("a|", ""));
    EXPECT_TRUE(Match("|a", "a"));
    EXPECT_TRUE(Match("|a", ""));
}

TEST_F(RegExpStandardComplianceTest, QuantifierOnAssertionDoesNotConsume) {
    // Lookahead assertions consume no characters, so (?=a)* matches empty string only
    EXPECT_TRUE(Match("^(?=a)*$", ""));
    // With full match semantics, (?=a)* doesn't match "a" because the lookahead
    // doesn't consume characters (end_pos=0 < str.size()=1)
    EXPECT_FALSE(Match("^(?=a)*$", "a"));
    // In partial search mode, it should find the zero-width match at position 0
    RegExpParser parser("(?=a)*");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);
    NFA nfa = NFABuilder::Build(ast.get());
    auto result = nfa.MatchEnd("a", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0);  // Zero-width match at position 0
}

TEST_F(RegExpStandardComplianceTest, DollarMatchesEndInMultiline) {
    RegExpParser parser("a$");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get(), false, true);
    auto result = nfa.MatchEnd("a\nb", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1);
}

TEST_F(RegExpStandardComplianceTest, CaretMatchesStartInMultiline) {
    RegExpParser parser("^b");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get(), false, true);
    auto result = nfa.MatchEnd("a\nb", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 3);
}

TEST_F(RegExpStandardComplianceTest, UnicodeSetsFlagRejectsU) {
    EXPECT_FALSE(RegExpObject::ValidateFlags("uv").empty());
    EXPECT_FALSE(RegExpObject::ValidateFlags("vu").empty());
}

}  // namespace mjs::test

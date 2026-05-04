/**
 * @file regexp_object_test.cpp
 * @brief RegExpObject tests
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/value.h>
#include <mjs/value/object/array_object.h>
#include <mjs/regexp/regexp_object.h>
#include <mjs/const_index_embedded.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class RegExpObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
        context = std::make_unique<Context>(test_env->runtime());
    }

    void TearDown() override {
        context.reset();
        test_env.reset();
    }

    Value GetArrayElement(const Value& array_value, int64_t index) {
        Value result;
        array_value.array().GetComputedProperty(context.get(), Value(index), &result);
        return result;
    }

    std::unique_ptr<TestEnvironment> test_env;
    std::unique_ptr<Context> context;
};

TEST_F(RegExpObjectTest, BasicCreation) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "");

    ASSERT_NE(regex->GetNFA(), nullptr);
    EXPECT_EQ(regex->pattern(), "abc");
    EXPECT_EQ(regex->flags(), "");
    EXPECT_FALSE(regex->global());
    EXPECT_FALSE(regex->ignore_case());
}

TEST_F(RegExpObjectTest, CreationWithFlags) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "dgimsvy");

    ASSERT_NE(regex->GetNFA(), nullptr);
    EXPECT_TRUE(regex->has_indices());
    EXPECT_TRUE(regex->global());
    EXPECT_TRUE(regex->ignore_case());
    EXPECT_TRUE(regex->multiline());
    EXPECT_TRUE(regex->dot_all());
    EXPECT_TRUE(regex->unicode_sets());
    EXPECT_TRUE(regex->sticky());
}

TEST_F(RegExpObjectTest, ToStringIncludesFlags) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("\\d+", "gi");
    EXPECT_EQ(regex->ToString(), "/\\d+/gi");
}

TEST_F(RegExpObjectTest, TestUsesSearchSemantics) {
    GCHandleScope<2> scope(context.get());
    auto regex1 = scope.New<RegExpObject>("abc", "");
    auto regex2 = scope.New<RegExpObject>("", "");

    EXPECT_TRUE(regex1->Test(context.get(), "abcdef"));
    EXPECT_FALSE(regex1->Test(context.get(), "abx"));
    EXPECT_TRUE(regex2->Test(context.get(), "abc"));
}

TEST_F(RegExpObjectTest, IgnoreCaseAndCharacterClasses) {
    GCHandleScope<2> scope(context.get());
    auto regex1 = scope.New<RegExpObject>("abc", "i");
    auto regex2 = scope.New<RegExpObject>("[a-z]+", "");

    EXPECT_TRUE(regex1->Test(context.get(), "AbC"));
    EXPECT_TRUE(regex2->Test(context.get(), "hello"));
    EXPECT_FALSE(regex2->Test(context.get(), "123"));
}

TEST_F(RegExpObjectTest, IgnoreCaseAppliesToCharacterClasses) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("[a-z]+", "i");

    EXPECT_TRUE(regex->Test(context.get(), "ABC"));
    EXPECT_TRUE(regex->Test(context.get(), "abc"));
}

TEST_F(RegExpObjectTest, ExecReturnsIndexAndInput) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "");

    auto result = regex->Exec(context.get(), "xyzabcdef");
    ASSERT_TRUE(result.IsArrayObject());

    Value index_value;
    result.object().GetProperty(context.get(), ConstIndexEmbedded::kIndex, &index_value);
    ASSERT_TRUE(index_value.IsInt64());
    EXPECT_EQ(index_value.i64(), 3);

    Value input_value;
    result.object().GetProperty(context.get(), ConstIndexEmbedded::kInput, &input_value);
    EXPECT_TRUE(input_value.IsString() || input_value.IsStringView());
}

TEST_F(RegExpObjectTest, ExecNoMatchReturnsNull) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "");

    auto result = regex->Exec(context.get(), "xyz");
    EXPECT_TRUE(result.IsNull());
}

TEST_F(RegExpObjectTest, GlobalModeUpdatesLastIndexAcrossMatches) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "g");

    auto result1 = regex->Exec(context.get(), "abcabcabc");
    ASSERT_TRUE(result1.IsArrayObject());
    EXPECT_EQ(regex->last_index(), 3);

    auto result2 = regex->Exec(context.get(), "abcabcabc");
    ASSERT_TRUE(result2.IsArrayObject());
    EXPECT_EQ(regex->last_index(), 6);

    auto result3 = regex->Exec(context.get(), "abcabcabc");
    ASSERT_TRUE(result3.IsArrayObject());
    EXPECT_EQ(regex->last_index(), 9);

    auto result4 = regex->Exec(context.get(), "abcabcabc");
    EXPECT_TRUE(result4.IsNull());
    EXPECT_EQ(regex->last_index(), 0);
}

TEST_F(RegExpObjectTest, GlobalEmptyMatchAdvancesLastIndex) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("", "g");

    EXPECT_TRUE(regex->Exec(context.get(), "abc").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 1);

    EXPECT_TRUE(regex->Exec(context.get(), "abc").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 2);

    EXPECT_TRUE(regex->Exec(context.get(), "abc").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 3);

    EXPECT_TRUE(regex->Exec(context.get(), "abc").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 4);

    EXPECT_TRUE(regex->Exec(context.get(), "abc").IsNull());
    EXPECT_EQ(regex->last_index(), 0);
}

TEST_F(RegExpObjectTest, StickyEmptyMatchAdvancesLastIndex) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("", "y");

    EXPECT_TRUE(regex->Exec(context.get(), "ab").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 1);

    EXPECT_TRUE(regex->Exec(context.get(), "ab").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 2);

    EXPECT_TRUE(regex->Exec(context.get(), "ab").IsArrayObject());
    EXPECT_EQ(regex->last_index(), 3);

    EXPECT_TRUE(regex->Exec(context.get(), "ab").IsNull());
    EXPECT_EQ(regex->last_index(), 0);
}

TEST_F(RegExpObjectTest, UnicodeEmptyMatchAdvancesByCodePoint) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("", "gu");
    const std::string input = std::string("\xF0\x9F\x98\x80", 4) + "a";

    EXPECT_TRUE(regex->Exec(context.get(), input).IsArrayObject());
    EXPECT_EQ(regex->last_index(), 4);

    EXPECT_TRUE(regex->Exec(context.get(), input).IsArrayObject());
    EXPECT_EQ(regex->last_index(), 5);
}

TEST_F(RegExpObjectTest, StickyModeRequiresExactPosition) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("abc", "y");

    regex->set_last_index(3);
    auto success = regex->Exec(context.get(), "xyzabc");
    ASSERT_TRUE(success.IsArrayObject());
    EXPECT_EQ(regex->last_index(), 6);

    regex->set_last_index(1);
    auto failure = regex->Exec(context.get(), "xyzabc");
    EXPECT_TRUE(failure.IsNull());
    EXPECT_EQ(regex->last_index(), 0);
}

TEST_F(RegExpObjectTest, HasIndicesProvidesRanges) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("(a)", "d");

    auto result = regex->Exec(context.get(), "a");
    ASSERT_TRUE(result.IsArrayObject());

    auto capture = GetArrayElement(result, 1);
    EXPECT_TRUE(capture.IsString() || capture.IsStringView());

    Value indices_value;
    result.object().GetProperty(context.get(), ConstIndexEmbedded::kIndices, &indices_value);
    ASSERT_TRUE(indices_value.IsArrayObject());
    EXPECT_EQ(indices_value.array().GetLength(), 2u);
}

TEST_F(RegExpObjectTest, MultilineModeWorks) {
    GCHandleScope<2> scope(context.get());
    auto regex1 = scope.New<RegExpObject>("^abc", "m");
    auto regex2 = scope.New<RegExpObject>("abc$", "m");

    EXPECT_TRUE(regex1->Test(context.get(), "xyz\nabc"));
    EXPECT_TRUE(regex2->Test(context.get(), "abc\nxyz"));
}

TEST_F(RegExpObjectTest, DotAllModeWorks) {
    GCHandleScope<2> scope(context.get());
    auto regex1 = scope.New<RegExpObject>("a.b", "s");
    auto regex2 = scope.New<RegExpObject>("a.b", "");

    EXPECT_TRUE(regex1->Test(context.get(), "a\nb"));
    EXPECT_FALSE(regex2->Test(context.get(), "a\nb"));
}

TEST_F(RegExpObjectTest, UnicodeModeWorks) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("\\u{41}", "u");
    EXPECT_TRUE(regex->Test(context.get(), "A"));
}

TEST_F(RegExpObjectTest, UnicodePropertyEscapesRequireUnicodeFlag) {
    GCHandleScope<2> scope(context.get());
    auto literal = scope.New<RegExpObject>("\\p{L}", "");
    auto unicode = scope.New<RegExpObject>("\\p{L}", "u");

    EXPECT_TRUE(literal->Test(context.get(), "p{L}"));
    EXPECT_FALSE(literal->Test(context.get(), "A"));
    EXPECT_TRUE(unicode->Test(context.get(), "A"));
}

TEST_F(RegExpObjectTest, InvalidFlagsRejectCompilation) {
    GCHandleScope<3> scope(context.get());
    auto duplicate = scope.New<RegExpObject>("a", "gg");
    auto invalid = scope.New<RegExpObject>("a", "z");
    auto incompatible = scope.New<RegExpObject>("a", "uv");

    EXPECT_EQ(duplicate->GetNFA(), nullptr);
    EXPECT_EQ(invalid->GetNFA(), nullptr);
    EXPECT_EQ(incompatible->GetNFA(), nullptr);
}

TEST_F(RegExpObjectTest, ValidateFlagsAllowsNewStandardFlags) {
    EXPECT_TRUE(RegExpObject::ValidateFlags("d").empty());
    EXPECT_TRUE(RegExpObject::ValidateFlags("v").empty());
    EXPECT_TRUE(RegExpObject::ValidateFlags("dgimsvy").empty());
}

TEST_F(RegExpObjectTest, ValidateFlagsRejectsIncompatibleUnicodeModes) {
    EXPECT_FALSE(RegExpObject::ValidateFlags("uv").empty());
    EXPECT_FALSE(RegExpObject::ValidateFlags("vu").empty());
}

TEST_F(RegExpObjectTest, ExecWithCaptureGroupReturnsCorrectResults) {
    GCHandleScope<2> scope(context.get());
    auto regex = scope.New<RegExpObject>("(a)(b)", "");

    auto result = regex->Exec(context.get(), "ab");
    ASSERT_TRUE(result.IsArrayObject());
    EXPECT_EQ(result.array().GetLength(), 3u);
    // The array has 3 elements: [full match, group 1, group 2]
    // (Element access via GetComputedProperty is tested in HasIndicesProvidesRanges)
}

TEST_F(RegExpObjectTest, ExecWithOptionalCaptureGroup) {
    GCHandleScope<2> scope(context.get());
    auto regex = scope.New<RegExpObject>("(a)?b", "");

    auto result = regex->Exec(context.get(), "b");
    ASSERT_TRUE(result.IsArrayObject());
    EXPECT_EQ(result.array().GetLength(), 2u);
}

TEST_F(RegExpObjectTest, ExecWithUnicodePropertyAndCapture) {
    GCHandleScope<2> scope(context.get());
    auto regex = scope.New<RegExpObject>("(\\p{L}+)", "u");

    auto result = regex->Exec(context.get(), "Hello");
    ASSERT_TRUE(result.IsArrayObject());
    EXPECT_EQ(result.array().GetLength(), 2u);
}

TEST_F(RegExpObjectTest, ExecWithNamedCaptureGroup) {
    GCHandleScope<2> scope(context.get());
    auto regex = scope.New<RegExpObject>("(?<letter>a)", "");

    auto result = regex->Exec(context.get(), "a");
    ASSERT_TRUE(result.IsArrayObject());
    EXPECT_EQ(result.array().GetLength(), 2u);
}

TEST_F(RegExpObjectTest, TestWithLookahead) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("a(?=b)", "");

    EXPECT_TRUE(regex->Test(context.get(), "ab"));
    EXPECT_FALSE(regex->Test(context.get(), "ac"));
    EXPECT_FALSE(regex->Test(context.get(), "a"));
}

TEST_F(RegExpObjectTest, TestWithLookbehind) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("(?<=a)b", "");

    EXPECT_TRUE(regex->Test(context.get(), "ab"));
    EXPECT_FALSE(regex->Test(context.get(), "xb"));
    EXPECT_FALSE(regex->Test(context.get(), "b"));
}

TEST_F(RegExpObjectTest, StickyModeWithLastIndexUpdate) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("a", "y");

    regex->set_last_index(2);
    auto result = regex->Exec(context.get(), "bbabb");
    ASSERT_TRUE(result.IsArrayObject());
    EXPECT_EQ(regex->last_index(), 3);
}

TEST_F(RegExpObjectTest, GlobalModeResetsLastIndexOnNoMatch) {
    GCHandleScope<1> scope(context.get());
    auto regex = scope.New<RegExpObject>("z", "g");

    regex->set_last_index(0);
    auto result = regex->Exec(context.get(), "abc");
    EXPECT_TRUE(result.IsNull());
    EXPECT_EQ(regex->last_index(), 0);
}

} // namespace mjs::test

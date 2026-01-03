/**
 * @file name_mangler_test.cpp
 * @brief NameMangler单元测试
 */

#include <gtest/gtest.h>

#include "cpp_gen/name_mangler.h"

using namespace mjs::compiler::cpp_gen;

TEST(NameManglerTest, RegularIdentifiers) {
    NameMangler mangler;

    EXPECT_EQ(mangler.Mangle("variable"), "variable");
    EXPECT_EQ(mangler.Mangle("myVar"), "myVar");
    EXPECT_EQ(mangler.Mangle("x"), "x");
    EXPECT_EQ(mangler.Mangle("_private"), "_private");
}

TEST(NameManglerTest, CppKeywords) {
    NameMangler mangler;

    EXPECT_EQ(mangler.Mangle("int"), "js_int_");
    EXPECT_EQ(mangler.Mangle("class"), "js_class_");
    EXPECT_EQ(mangler.Mangle("return"), "js_return_");
    EXPECT_EQ(mangler.Mangle("if"), "js_if_");
    EXPECT_EQ(mangler.Mangle("for"), "js_for_");
}

TEST(NameManglerTest, NeedsMangling) {
    NameMangler mangler;

    EXPECT_TRUE(mangler.NeedsMangling("int"));
    EXPECT_TRUE(mangler.NeedsMangling("class"));
    EXPECT_FALSE(mangler.NeedsMangling("variable"));
    EXPECT_FALSE(mangler.NeedsMangling("myVar"));
}

TEST(NameManglerTest, ReservedWords) {
    NameMangler mangler;

    mangler.AddReservedWord("std");
    mangler.AddReservedWord("string");

    EXPECT_EQ(mangler.Mangle("std"), "js_std");
    EXPECT_EQ(mangler.Mangle("string"), "js_string");
}

TEST(NameManglerTest, DigitStartIdentifiers) {
    NameMangler mangler;

    EXPECT_EQ(mangler.Mangle("123abc"), "_js_123abc");
    EXPECT_EQ(mangler.Mangle("0"), "_js_0");
}

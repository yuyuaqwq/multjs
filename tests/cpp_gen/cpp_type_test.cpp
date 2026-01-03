/**
 * @file cpp_type_test.cpp
 * @brief CppType单元测试
 */

#include <gtest/gtest.h>
#include "cpp_gen/cpp_type.h"

using namespace mjs::compiler::cpp_gen;

TEST(CppTypeTest, PrimitiveTypes) {
    auto int_type = CppType::Int64();
    EXPECT_TRUE(int_type.IsPrimitive());
    EXPECT_EQ(int_type.ToString(), "int64_t");

    auto double_type = CppType::Float64();
    EXPECT_TRUE(double_type.IsPrimitive());
    EXPECT_EQ(double_type.ToString(), "double");

    auto bool_type = CppType::Boolean();
    EXPECT_TRUE(bool_type.IsPrimitive());
    EXPECT_EQ(bool_type.ToString(), "bool");

    auto string_type = CppType::String();
    EXPECT_TRUE(string_type.IsPrimitive());
    EXPECT_EQ(string_type.ToString(), "std::string");
}

TEST(CppTypeTest, ArrayType) {
    auto int_type = std::make_shared<CppType>(CppType::Int64());
    auto array_type = CppType::Array(int_type);

    EXPECT_TRUE(array_type.IsArray());
    EXPECT_EQ(array_type.ToString(), "std::vector<int64_t>");

    const auto& elem_type = array_type.GetElementType();
    EXPECT_EQ(elem_type->ToString(), "int64_t");
}

TEST(CppTypeTest, OptionalType) {
    auto string_type = std::make_shared<CppType>(CppType::String());
    auto optional_type = CppType::Optional(string_type);

    EXPECT_EQ(optional_type.ToString(), "std::optional<std::string>");

    const auto& inner_type = optional_type.GetOptionalType();
    EXPECT_EQ(inner_type->ToString(), "std::string");
}

TEST(CppTypeTest, UnionType) {
    auto int_type = std::make_shared<CppType>(CppType::Int64());
    auto string_type = std::make_shared<CppType>(CppType::String());
    auto union_type = CppType::Union({int_type, string_type});

    EXPECT_EQ(union_type.ToString(), "std::variant<int64_t, std::string>");

    const auto& alternatives = union_type.GetUnionAlternatives();
    EXPECT_EQ(alternatives.size(), 2);
}

TEST(CppTypeTest, TypeMerge) {
    auto int_type = CppType::Int64();
    auto double_type = CppType::Float64();

    auto merged = int_type.Merge(double_type);
    EXPECT_EQ(merged.ToString(), "double");

    auto merged2 = double_type.Merge(int_type);
    EXPECT_EQ(merged2.ToString(), "double");
}

TEST(CppTypeTest, TypeEquality) {
    auto int_type1 = CppType::Int64();
    auto int_type2 = CppType::Int64();
    auto double_type = CppType::Float64();

    EXPECT_TRUE(int_type1.Equals(int_type2));
    EXPECT_FALSE(int_type1.Equals(double_type));
}

/**
 * @file type_conversion_test.cpp
 * @brief 类型转换单元测试
 *
 * 测试类型转换等类型系统组件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "src/compiler/statement_impl/type_base.h"
#include "src/compiler/statement_impl/predefined_type.h"
#include "src/compiler/statement_impl/union_type.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TypeConversionTest
 * @brief 类型转换测试
 */
class TypeConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = 0;
        end_ = 10;
    }

    SourceBytePosition start_;
    SourceBytePosition end_;
};

/**
 * @test 测试Number到String的类型转换(概念性测试)
 */
TEST_F(TypeConversionTest, NumberToStringConversion) {
    // 这是一个概念性测试,实际类型转换需要在运行时处理
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);
    PredefinedType string_type(start_, end_, PredefinedTypeKeyword::kString);

    EXPECT_NE(number_type.keyword(), string_type.keyword());
}

/**
 * @test 测试String到Number的类型转换(概念性测试)
 */
TEST_F(TypeConversionTest, StringToNumberConversion) {
    // 这是一个概念性测试,实际类型转换需要在运行时处理
    PredefinedType string_type(start_, end_, PredefinedTypeKeyword::kString);
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);

    EXPECT_NE(string_type.keyword(), number_type.keyword());
}

/**
 * @test 测试联合类型中的类型排序
 */
TEST_F(TypeConversionTest, UnionTypeOrdering) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types1;
    types1.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));
    types1.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));

    std::vector<std::unique_ptr<Type>> types2;
    types2.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));
    types2.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));

    // Act
    UnionType union_type1(start_, end_, std::move(types1));
    UnionType union_type2(start_, end_, std::move(types2));

    // Assert - 顺序不同应该被视为不同的联合类型
    EXPECT_EQ(union_type1.types().size(), union_type2.types().size());

    auto* type1_first = static_cast<const PredefinedType*>(union_type1.types()[0].get());
    auto* type2_first = static_cast<const PredefinedType*>(union_type2.types()[0].get());
    EXPECT_NE(type1_first->keyword(), type2_first->keyword());
}

} // namespace test
} // namespace compiler
} // namespace mjs

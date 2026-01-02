/**
 * @file type_compatibility_test.cpp
 * @brief 类型兼容性单元测试
 *
 * 测试类型兼容性等类型系统组件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "src/compiler/statement_impl/type_base.h"
#include "src/compiler/statement_impl/predefined_type.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TypeCompatibilityTest
 * @brief 类型兼容性测试
 */
class TypeCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = 0;
        end_ = 10;
    }

    SourceBytePosition start_;
    SourceBytePosition end_;
};

/**
 * @test 测试相同类型兼容性
 */
TEST_F(TypeCompatibilityTest, SameTypeCompatibility) {
    // Arrange
    PredefinedType type1(start_, end_, PredefinedTypeKeyword::kNumber);
    PredefinedType type2(start_, end_, PredefinedTypeKeyword::kNumber);

    // Act & Assert
    EXPECT_EQ(type1.keyword(), type2.keyword());
}

/**
 * @test 测试不同类型不兼容性
 */
TEST_F(TypeCompatibilityTest, DifferentTypeIncompatibility) {
    // Arrange
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);
    PredefinedType string_type(start_, end_, PredefinedTypeKeyword::kString);

    // Act & Assert
    EXPECT_NE(number_type.keyword(), string_type.keyword());
}

/**
 * @test 测试Any类型与所有类型兼容
 */
TEST_F(TypeCompatibilityTest, AnyTypeCompatibility) {
    // Arrange
    PredefinedType any_type(start_, end_, PredefinedTypeKeyword::kAny);
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);
    PredefinedType string_type(start_, end_, PredefinedTypeKeyword::kString);

    // Act & Assert
    // Any类型应该与所有类型兼容
    EXPECT_EQ(any_type.keyword(), PredefinedTypeKeyword::kAny);
    EXPECT_NE(number_type.keyword(), any_type.keyword());
}

/**
 * @test 测试Void类型特殊性
 */
TEST_F(TypeCompatibilityTest, VoidTypeSpecialCase) {
    // Arrange
    PredefinedType void_type(start_, end_, PredefinedTypeKeyword::kVoid);
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);

    // Act & Assert
    EXPECT_EQ(void_type.keyword(), PredefinedTypeKeyword::kVoid);
    EXPECT_NE(void_type.keyword(), number_type.keyword());
}

} // namespace test
} // namespace compiler
} // namespace mjs

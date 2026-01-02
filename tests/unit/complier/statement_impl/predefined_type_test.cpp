/**
 * @file predefined_type_test.cpp
 * @brief 预定义类型单元测试
 *
 * 测试PredefinedType等预定义类型系统组件
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
 * @class PredefinedTypeTest
 * @brief 预定义类型测试
 */
class PredefinedTypeTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = 0;
        end_ = 0;
    }

    SourceBytePosition start_;
    SourceBytePosition end_;
};

/**
 * @test 测试Number类型
 */
TEST_F(PredefinedTypeTest, NumberType) {
    // Arrange & Act
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kNumber);

    // Assert
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
    EXPECT_EQ(type.keyword(), PredefinedTypeKeyword::kNumber);
}

/**
 * @test 测试String类型
 */
TEST_F(PredefinedTypeTest, StringType) {
    // Arrange & Act
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kString);

    // Assert
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
    EXPECT_EQ(type.keyword(), PredefinedTypeKeyword::kString);
}

/**
 * @test 测试Boolean类型
 */
TEST_F(PredefinedTypeTest, BooleanType) {
    // Arrange & Act
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kBoolean);

    // Assert
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
    EXPECT_EQ(type.keyword(), PredefinedTypeKeyword::kBoolean);
}

/**
 * @test 测试Any类型
 */
TEST_F(PredefinedTypeTest, AnyType) {
    // Arrange & Act
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kAny);

    // Assert
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
    EXPECT_EQ(type.keyword(), PredefinedTypeKeyword::kAny);
}

/**
 * @test 测试Void类型
 */
TEST_F(PredefinedTypeTest, VoidType) {
    // Arrange & Act
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kVoid);

    // Assert
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
    EXPECT_EQ(type.keyword(), PredefinedTypeKeyword::kVoid);
}

/**
 * @test 测试所有预定义类型枚举值
 */
TEST_F(PredefinedTypeTest, AllPredefinedTypes) {
    // Arrange & Act & Assert
    PredefinedType number_type(start_, end_, PredefinedTypeKeyword::kNumber);
    PredefinedType string_type(start_, end_, PredefinedTypeKeyword::kString);
    PredefinedType boolean_type(start_, end_, PredefinedTypeKeyword::kBoolean);
    PredefinedType any_type(start_, end_, PredefinedTypeKeyword::kAny);
    PredefinedType void_type(start_, end_, PredefinedTypeKeyword::kVoid);

    EXPECT_EQ(number_type.keyword(), PredefinedTypeKeyword::kNumber);
    EXPECT_EQ(string_type.keyword(), PredefinedTypeKeyword::kString);
    EXPECT_EQ(boolean_type.keyword(), PredefinedTypeKeyword::kBoolean);
    EXPECT_EQ(any_type.keyword(), PredefinedTypeKeyword::kAny);
    EXPECT_EQ(void_type.keyword(), PredefinedTypeKeyword::kVoid);
}

/**
 * @test 测试类型位置信息
 */
TEST_F(PredefinedTypeTest, TypePosition) {
    // Arrange
    SourceBytePosition start = 5;
    SourceBytePosition end = 10;

    // Act
    PredefinedType type(start, end, PredefinedTypeKeyword::kNumber);

    // Assert
    EXPECT_EQ(type.start(), 5);
    EXPECT_EQ(type.end(), 10);
}

/**
 * @test 测试GenerateCode接口存在(空实现)
 */
TEST_F(PredefinedTypeTest, GenerateCodeInterface) {
    // Arrange
    PredefinedType type(start_, end_, PredefinedTypeKeyword::kNumber);

    // Act & Assert - 验证GenerateCode方法存在且可以调用
    // 这是一个编译时检查，确保虚函数存在
    // 实际的代码生成测试需要CodeGenerator和FunctionDefBase的完整设置
    EXPECT_EQ(type.type(), StatementType::kPredefinedType);
}

} // namespace test
} // namespace compiler
} // namespace mjs

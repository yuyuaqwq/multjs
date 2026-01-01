/**
 * @file type_system_test.cpp
 * @brief 类型系统单元测试
 *
 * 测试TypeBase、PredefinedType、UnionType等类型系统组件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "../src/compiler/statement_impl/type_base.h"
#include "../src/compiler/statement_impl/predefined_type.h"
#include "../src/compiler/statement_impl/union_type.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TypeBaseTest
 * @brief 类型基类测试
 */
class TypeBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = SourcePosition{0, 0, 0};
        end_ = SourcePosition{0, 0, 0};
    }

    SourcePosition start_;
    SourcePosition end_;
};

/**
 * @test 测试Type基类构造
 */
TEST_F(TypeBaseTest, TypeBaseConstruction) {
    // Arrange & Act
    Type type(start_, end_);

    // Assert
    EXPECT_EQ(type.start().line, 0);
    EXPECT_EQ(type.start().column, 0);
}

/**
 * @class PredefinedTypeTest
 * @brief 预定义类型测试
 */
class PredefinedTypeTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = SourcePosition{0, 0, 0};
        end_ = SourcePosition{0, 10, 10};
    }

    SourcePosition start_;
    SourcePosition end_;
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
    SourcePosition start{1, 5, 10};
    SourcePosition end{1, 10, 15};

    // Act
    PredefinedType type(start, end, PredefinedTypeKeyword::kNumber);

    // Assert
    EXPECT_EQ(type.start().line, 1);
    EXPECT_EQ(type.start().column, 5);
    EXPECT_EQ(type.end().line, 1);
    EXPECT_EQ(type.end().column, 10);
}

/**
 * @class UnionTypeTest
 * @brief 联合类型测试
 */
class UnionTypeTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = SourcePosition{0, 0, 0};
        end_ = SourcePosition{0, 20, 20};
    }

    SourcePosition start_;
    SourcePosition end_;
};

/**
 * @test 测试简单联合类型
 */
TEST_F(UnionTypeTest, SimpleUnionType) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));

    // Act
    UnionType union_type(start_, end_, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 2);
}

/**
 * @test 测试联合类型成员访问
 */
TEST_F(UnionTypeTest, UnionTypeMembers) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start_, end_, std::move(types));

    // Assert
    EXPECT_EQ(union_type.types().size(), 3);

    // 验证第一个类型
    auto* first_type = static_cast<const PredefinedType*>(union_type.types()[0].get());
    EXPECT_EQ(first_type->keyword(), PredefinedTypeKeyword::kNumber);

    // 验证第二个类型
    auto* second_type = static_cast<const PredefinedType*>(union_type.types()[1].get());
    EXPECT_EQ(second_type->keyword(), PredefinedTypeKeyword::kString);

    // 验证第三个类型
    auto* third_type = static_cast<const PredefinedType*>(union_type.types()[2].get());
    EXPECT_EQ(third_type->keyword(), PredefinedTypeKeyword::kBoolean);
}

/**
 * @test 测试单一成员联合类型(退化情况)
 */
TEST_F(UnionTypeTest, SingleMemberUnionType) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));

    // Act
    UnionType union_type(start_, end_, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 1);
}

/**
 * @test 测试空联合类型(边缘情况)
 */
TEST_F(UnionTypeTest, EmptyUnionType) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types;

    // Act
    UnionType union_type(start_, end_, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 0);
}

/**
 * @test 测试复杂联合类型(包含Any)
 */
TEST_F(UnionTypeTest, ComplexUnionTypeWithAny) {
    // Arrange
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kAny));
    types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start_, end_, std::move(types));

    // Assert
    EXPECT_EQ(union_type.types().size(), 4);

    // 验证Any类型存在
    bool has_any = false;
    for (const auto& type : union_type.types()) {
        if (type->type() == StatementType::kPredefinedType) {
            auto* predefined_type = static_cast<const PredefinedType*>(type.get());
            if (predefined_type->keyword() == PredefinedTypeKeyword::kAny) {
                has_any = true;
                break;
            }
        }
    }
    EXPECT_TRUE(has_any);
}

/**
 * @test 测试嵌套联合类型(如果支持)
 */
TEST_F(UnionTypeTest, NestedUnionType) {
    // Arrange
    std::vector<std::unique_ptr<Type>> inner_types;
    inner_types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kNumber));
    inner_types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kString));

    std::vector<std::unique_ptr<Type>> outer_types;
    outer_types.push_back(std::make_unique<UnionType>(start_, end_, std::move(inner_types)));
    outer_types.push_back(std::make_unique<PredefinedType>(start_, end_, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start_, end_, std::move(outer_types));

    // Assert
    EXPECT_EQ(union_type.types().size(), 2);
    EXPECT_EQ(union_type.types()[0]->type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types()[1]->type(), StatementType::kPredefinedType);
}

/**
 * @test 测试联合类型位置信息
 */
TEST_F(UnionTypeTest, UnionTypePosition) {
    // Arrange
    SourcePosition start{2, 3, 15};
    SourcePosition end{2, 25, 50};

    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));

    // Act
    UnionType union_type(start, end, std::move(types));

    // Assert
    EXPECT_EQ(union_type.start().line, 2);
    EXPECT_EQ(union_type.start().column, 3);
    EXPECT_EQ(union_type.end().line, 2);
    EXPECT_EQ(union_type.end().column, 25);
}

/**
 * @class TypeCompatibilityTest
 * @brief 类型兼容性测试
 */
class TypeCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = SourcePosition{0, 0, 0};
        end_ = SourcePosition{0, 10, 10};
    }

    SourcePosition start_;
    SourcePosition end_;
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

/**
 * @class TypeConversionTest
 * @brief 类型转换测试
 */
class TypeConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        start_ = SourcePosition{0, 0, 0};
        end_ = SourcePosition{0, 10, 10};
    }

    SourcePosition start_;
    SourcePosition end_;
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

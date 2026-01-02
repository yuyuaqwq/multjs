/**
 * @file union_type_test.cpp
 * @brief 联合类型测试
 *
 * 测试联合类型功能,包括:
 * - UnionType (联合类型)
 * - 联合类型的构造
 * - 联合类型的成员访问
 * - 联合类型的解析
 * - 多个类型的组合
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include <mjs/error.h>

#include "src/compiler/lexer.h"
#include "src/compiler/statement_impl/union_type.h"
#include "src/compiler/statement_impl/named_type.h"
#include "src/compiler/statement_impl/predefined_type.h"
#include "src/compiler/statement_impl/type_base.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class UnionTypeTest
 * @brief 联合类型测试类
 */
class UnionTypeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：创建NamedType对象
     * @param name 类型名称
     * @return NamedType对象的唯一指针
     */
    std::unique_ptr<NamedType> CreateNamedType(const std::string& name) {
        return std::make_unique<NamedType>(0, name.length(), std::string(name));
    }

    /**
     * @brief 辅助方法：创建UnionType对象
     * @param types 类型列表
     * @return UnionType对象的唯一指针
     */
    std::unique_ptr<UnionType> CreateUnionType(std::vector<std::unique_ptr<Type>>&& types) {
        size_t total_length = 0;
        for (const auto& type : types) {
            total_length += type->end() - type->start();
        }
        return std::make_unique<UnionType>(0, total_length, std::move(types));
    }
};

// ============================================================================
// UnionType 构造函数测试
// ============================================================================

/**
 * @test 测试UnionType构造函数
 */
TEST_F(UnionTypeTest, Constructor) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 2);
}

/**
 * @test 测试UnionType的position信息
 */
TEST_F(UnionTypeTest, Position) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->start(), 0);
    EXPECT_GT(union_type->end(), 0);
}

/**
 * @test 测试UnionType的type方法
 */
TEST_F(UnionTypeTest, StatementType) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("boolean"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->type(), StatementType::kUnionType);
}

// ============================================================================
// UnionType types() 方法测试
// ============================================================================

/**
 * @test 测试types()方法返回类型列表
 */
TEST_F(UnionTypeTest, TypesMethod) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("boolean"));

    auto union_type = CreateUnionType(std::move(types));

    const auto& result_types = union_type->types();
    EXPECT_EQ(result_types.size(), 3);

    // 验证第一个类型
    auto* type1 = dynamic_cast<NamedType*>(result_types[0].get());
    ASSERT_NE(type1, nullptr);
    EXPECT_EQ(type1->name(), "string");

    // 验证第二个类型
    auto* type2 = dynamic_cast<NamedType*>(result_types[1].get());
    ASSERT_NE(type2, nullptr);
    EXPECT_EQ(type2->name(), "number");

    // 验证第三个类型
    auto* type3 = dynamic_cast<NamedType*>(result_types[2].get());
    ASSERT_NE(type3, nullptr);
    EXPECT_EQ(type3->name(), "boolean");
}

/**
 * @test 测试types()返回const引用
 */
TEST_F(UnionTypeTest, TypesReturnsConstReference) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));

    auto union_type = CreateUnionType(std::move(types));

    const auto& ref1 = union_type->types();
    const auto& ref2 = union_type->types();
    EXPECT_EQ(&ref1, &ref2); // 应该返回同一个引用
}

// ============================================================================
// 基本联合类型测试
// ============================================================================

/**
 * @test 测试两个类型的联合
 */
TEST_F(UnionTypeTest, TwoTypesUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 2);
}

/**
 * @test 测试三个类型的联合
 */
TEST_F(UnionTypeTest, ThreeTypesUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("boolean"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 3);
}

/**
 * @test 测试多个类型的联合
 */
TEST_F(UnionTypeTest, MultipleTypesUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("boolean"));
    types.push_back(CreateNamedType("void"));
    types.push_back(CreateNamedType("any"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 5);
}

// ============================================================================
// 常用联合类型组合测试
// ============================================================================

/**
 * @test 测试string | number联合类型
 */
TEST_F(UnionTypeTest, StringOrNumber) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "number");
}

/**
 * @test 测试string | number | boolean联合类型
 */
TEST_F(UnionTypeTest, StringOrNumberOrBoolean) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("boolean"));

    auto union_type = CreateUnionType(std::move(types));

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());
    auto* type3 = dynamic_cast<NamedType*>(union_type->types()[2].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);
    ASSERT_NE(type3, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "number");
    EXPECT_EQ(type3->name(), "boolean");
}

/**
 * @test 测试null | undefined联合类型
 */
TEST_F(UnionTypeTest, NullOrUndefined) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("null"));
    types.push_back(CreateNamedType("undefined"));

    auto union_type = CreateUnionType(std::move(types));

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "null");
    EXPECT_EQ(type2->name(), "undefined");
}

/**
 * @test 测试自定义类型联合
 */
TEST_F(UnionTypeTest, CustomTypesUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("Error"));
    types.push_back(CreateNamedType("SyntaxError"));
    types.push_back(CreateNamedType("TypeError"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 3);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());
    auto* type3 = dynamic_cast<NamedType*>(union_type->types()[2].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);
    ASSERT_NE(type3, nullptr);

    EXPECT_EQ(type1->name(), "Error");
    EXPECT_EQ(type2->name(), "SyntaxError");
    EXPECT_EQ(type3->name(), "TypeError");
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试单一类型的联合
 */
TEST_F(UnionTypeTest, SingleTypeUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 1);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    ASSERT_NE(type1, nullptr);
    EXPECT_EQ(type1->name(), "string");
}

/**
 * @test 测试空联合类型(边界情况)
 */
TEST_F(UnionTypeTest, EmptyUnion) {
    std::vector<std::unique_ptr<Type>> types;
    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 0);
}

/**
 * @test 测试包含any的联合类型
 */
TEST_F(UnionTypeTest, UnionWithAny) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("any"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 3);

    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());
    ASSERT_NE(type2, nullptr);
    EXPECT_EQ(type2->name(), "any");
}

// ============================================================================
// 复杂联合类型测试
// ============================================================================

/**
 * @test 测试包含很多类型的联合
 */
TEST_F(UnionTypeTest, LargeUnion) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("boolean"));
    types.push_back(CreateNamedType("null"));
    types.push_back(CreateNamedType("undefined"));
    types.push_back(CreateNamedType("void"));
    types.push_back(CreateNamedType("object"));
    types.push_back(CreateNamedType("function"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 8);
}

/**
 * @test 测试包含泛型类型的联合
 */
TEST_F(UnionTypeTest, UnionWithGenericTypes) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("Array"));
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("Promise"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 3);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type3 = dynamic_cast<NamedType*>(union_type->types()[2].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type3, nullptr);

    EXPECT_EQ(type1->name(), "Array");
    EXPECT_EQ(type3->name(), "Promise");
}

// ============================================================================
// 可选类型测试
// ============================================================================

/**
 * @test 测试可选类型(T | null)
 */
TEST_F(UnionTypeTest, OptionalTypeNull) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("null"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 2);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "null");
}

/**
 * @test 测试可选类型(T | undefined)
 */
TEST_F(UnionTypeTest, OptionalTypeUndefined) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("number"));
    types.push_back(CreateNamedType("undefined"));

    auto union_type = CreateUnionType(std::move(types));

    EXPECT_EQ(union_type->types().size(), 2);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "number");
    EXPECT_EQ(type2->name(), "undefined");
}

// ============================================================================
// 移动语义测试
// ============================================================================

/**
 * @test 测试UnionType移动构造
 */
TEST_F(UnionTypeTest, MoveSemantics) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));
    types.push_back(CreateNamedType("number"));

    auto union_type = CreateUnionType(std::move(types));

    // 验证对象被正确移动
    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 2);
}

// ============================================================================
// 代码生成接口测试
// ============================================================================

/**
 * @test 测试GenerateCode接口存在
 */
TEST_F(UnionTypeTest, CodeGenerationInterface) {
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(CreateNamedType("string"));

    auto union_type = CreateUnionType(std::move(types));

    // 验证对象可以被正确创建,且接口存在
    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->type(), StatementType::kUnionType);
    // 注意:实际的代码生成测试需要CodeGenerator和FunctionDef的完整设置
    // 联合类型在运行时不生成代码,只在编译时用于类型检查
}

// ============================================================================
// 基于 PredefinedType 的联合类型测试
// ============================================================================

/**
 * @test 测试简单联合类型(PredefinedType)
 */
TEST_F(UnionTypeTest, SimpleUnionTypeWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));

    // Act
    UnionType union_type(start, end, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 2);
}

/**
 * @test 测试联合类型成员访问(PredefinedType)
 */
TEST_F(UnionTypeTest, UnionTypeMembersWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start, end, std::move(types));

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
 * @test 测试单一成员联合类型退化情况(PredefinedType)
 */
TEST_F(UnionTypeTest, SingleMemberUnionTypeWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));

    // Act
    UnionType union_type(start, end, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 1);
}

/**
 * @test 测试空联合类型边缘情况(PredefinedType)
 */
TEST_F(UnionTypeTest, EmptyUnionTypeWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> types;

    // Act
    UnionType union_type(start, end, std::move(types));

    // Assert
    EXPECT_EQ(union_type.type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types().size(), 0);
}

/**
 * @test 测试复杂联合类型包含Any(PredefinedType)
 */
TEST_F(UnionTypeTest, ComplexUnionTypeWithAnyPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kAny));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start, end, std::move(types));

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
 * @test 测试嵌套联合类型(PredefinedType)
 */
TEST_F(UnionTypeTest, NestedUnionTypeWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::vector<std::unique_ptr<Type>> inner_types;
    inner_types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    inner_types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));

    std::vector<std::unique_ptr<Type>> outer_types;
    outer_types.push_back(std::make_unique<UnionType>(start, end, std::move(inner_types)));
    outer_types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kBoolean));

    // Act
    UnionType union_type(start, end, std::move(outer_types));

    // Assert
    EXPECT_EQ(union_type.types().size(), 2);
    EXPECT_EQ(union_type.types()[0]->type(), StatementType::kUnionType);
    EXPECT_EQ(union_type.types()[1]->type(), StatementType::kPredefinedType);
}

/**
 * @test 测试联合类型位置信息(PredefinedType)
 */
TEST_F(UnionTypeTest, UnionTypePositionWithPredefinedType) {
    // Arrange
    SourceBytePosition start = 3;
    SourceBytePosition end = 25;

    std::vector<std::unique_ptr<Type>> types;
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kNumber));
    types.push_back(std::make_unique<PredefinedType>(start, end, PredefinedTypeKeyword::kString));

    // Act
    UnionType union_type(start, end, std::move(types));

    // Assert
    EXPECT_EQ(union_type.start(), 3);
    EXPECT_EQ(union_type.end(), 25);
}

// ============================================================================
// ParseUnionType 方法测试
// ============================================================================

/**
 * @test 测试ParseUnionType解析简单联合类型
 */
TEST_F(UnionTypeTest, ParseUnionTypeSimple) {
    Lexer lexer("string | number");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->type(), StatementType::kUnionType);
    EXPECT_EQ(union_type->types().size(), 2);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "number");
}

/**
 * @test 测试ParseUnionType解析多个类型
 */
TEST_F(UnionTypeTest, ParseUnionTypeMultiple) {
    Lexer lexer("string | number | boolean");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 3);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());
    auto* type3 = dynamic_cast<NamedType*>(union_type->types()[2].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);
    ASSERT_NE(type3, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "number");
    EXPECT_EQ(type3->name(), "boolean");
}

/**
 * @test 测试ParseUnionType解析单一类型(没有|操作符)
 */
TEST_F(UnionTypeTest, ParseUnionTypeSingle) {
    Lexer lexer("string");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 1);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    ASSERT_NE(type1, nullptr);
    EXPECT_EQ(type1->name(), "string");
}

/**
 * @test 测试ParseUnionType在第一个类型不是identifier时抛出异常
 */
TEST_F(UnionTypeTest, ParseUnionTypeThrowsOnInvalidFirstType) {
    Lexer lexer("| string");

    EXPECT_THROW(
        UnionType::ParseUnionType(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试ParseUnionType在|后不是identifier时抛出异常
 */
TEST_F(UnionTypeTest, ParseUnionTypeThrowsOnInvalidTypeAfterPipe) {
    Lexer lexer("string |");

    EXPECT_THROW(
        UnionType::ParseUnionType(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试ParseUnionType在|后是非法字符时抛出异常
 */
TEST_F(UnionTypeTest, ParseUnionTypeThrowsOnInvalidCharAfterPipe) {
    Lexer lexer("string | @");

    EXPECT_THROW(
        UnionType::ParseUnionType(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试ParseUnionType在空输入时抛出异常
 */
TEST_F(UnionTypeTest, ParseUnionTypeThrowsOnEmptyInput) {
    Lexer lexer("");

    EXPECT_THROW(
        UnionType::ParseUnionType(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试ParseUnionType位置信息
 */
TEST_F(UnionTypeTest, ParseUnionTypePosition) {
    Lexer lexer("string | number");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->start(), 0);
    EXPECT_GT(union_type->end(), union_type->start());
}

/**
 * @test 测试ParseUnionType解析自定义类型
 */
TEST_F(UnionTypeTest, ParseUnionTypeCustomTypes) {
    Lexer lexer("Error | TypeError | SyntaxError");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 3);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());
    auto* type3 = dynamic_cast<NamedType*>(union_type->types()[2].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);
    ASSERT_NE(type3, nullptr);

    EXPECT_EQ(type1->name(), "Error");
    EXPECT_EQ(type2->name(), "TypeError");
    EXPECT_EQ(type3->name(), "SyntaxError");
}

/**
 * @test 测试ParseUnionType解析包含undefined的联合类型
 */
TEST_F(UnionTypeTest, ParseUnionTypeWithUndefined) {
    Lexer lexer("string | undefinedName");
    auto union_type = UnionType::ParseUnionType(&lexer);

    ASSERT_NE(union_type, nullptr);
    EXPECT_EQ(union_type->types().size(), 2);

    auto* type1 = dynamic_cast<NamedType*>(union_type->types()[0].get());
    auto* type2 = dynamic_cast<NamedType*>(union_type->types()[1].get());

    ASSERT_NE(type1, nullptr);
    ASSERT_NE(type2, nullptr);

    EXPECT_EQ(type1->name(), "string");
    EXPECT_EQ(type2->name(), "undefinedName");
}

} // namespace test
} // namespace compiler
} // namespace mjs

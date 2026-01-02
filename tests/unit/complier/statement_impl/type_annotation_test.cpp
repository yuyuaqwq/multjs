/**
 * @file type_annotation_test.cpp
 * @brief 类型注解测试
 *
 * 测试类型注解功能,包括:
 * - TypeAnnotation (类型注解)
 * - NamedType (命名类型)
 * - 类型注解的解析
 * - 类型注解的构造
 * - 类型注解的值获取
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/error.h>

#include "src/compiler/lexer.h"
#include "src/compiler/statement_impl/type_annotation.h"
#include "src/compiler/statement_impl/named_type.h"
#include "src/compiler/statement_impl/type_base.h"
#include "src/compiler/statement_impl/union_type.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TypeAnnotationTest
 * @brief 类型注解测试类
 */
class TypeAnnotationTest : public ::testing::Test {
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
     * @brief 辅助方法：创建TypeAnnotation对象
     * @param type 类型对象
     * @return TypeAnnotation对象的唯一指针
     */
    std::unique_ptr<TypeAnnotation> CreateTypeAnnotation(std::unique_ptr<Type>&& type) {
        return std::make_unique<TypeAnnotation>(0, 10, std::move(type));
    }
};

// ============================================================================
// TypeAnnotation 构造函数测试
// ============================================================================

/**
 * @test 测试TypeAnnotation构造函数
 */
TEST_F(TypeAnnotationTest, Constructor) {
    auto named_type = CreateNamedType("string");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    ASSERT_NE(annotation, nullptr);
    EXPECT_NE(annotation->type_p(), nullptr);
}

/**
 * @test 测试TypeAnnotation的position信息
 */
TEST_F(TypeAnnotationTest, Position) {
    auto named_type = CreateNamedType("number");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    EXPECT_EQ(annotation->start(), 0);
    EXPECT_EQ(annotation->end(), 10);
}

/**
 * @test 测试TypeAnnotation的type方法
 */
TEST_F(TypeAnnotationTest, StatementType) {
    auto named_type = CreateNamedType("boolean");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    EXPECT_EQ(annotation->type(), StatementType::kTypeAnnotation);
}

// ============================================================================
// TypeAnnotation type_p() 方法测试
// ============================================================================

/**
 * @test 测试type_p()方法返回类型
 */
TEST_F(TypeAnnotationTest, TypePMethod) {
    auto named_type = CreateNamedType("MyType");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    const auto& type_p = annotation->type_p();
    ASSERT_NE(type_p, nullptr);

    // 验证是NamedType
    auto* named_type_ptr = dynamic_cast<NamedType*>(type_p.get());
    ASSERT_NE(named_type_ptr, nullptr);
    EXPECT_EQ(named_type_ptr->name(), "MyType");
}

/**
 * @test 测试type_p()返回const引用
 */
TEST_F(TypeAnnotationTest, TypePReturnsConstReference) {
    auto named_type = CreateNamedType("TestType");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    const auto& ref1 = annotation->type_p();
    const auto& ref2 = annotation->type_p();
    EXPECT_EQ(&ref1, &ref2); // 应该返回同一个引用
}

// ============================================================================
// NamedType 测试
// ============================================================================

/**
 * @test 测试NamedType构造函数
 */
TEST_F(TypeAnnotationTest, NamedTypeConstructor) {
    auto named_type = CreateNamedType("string");

    ASSERT_NE(named_type, nullptr);
    EXPECT_EQ(named_type->name(), "string");
}

/**
 * @test 测试NamedType的position信息
 */
TEST_F(TypeAnnotationTest, NamedTypePosition) {
    auto named_type = CreateNamedType("number");

    EXPECT_EQ(named_type->start(), 0);
    EXPECT_EQ(named_type->end(), 6); // "number"长度
}

/**
 * @test 测试NamedType的type方法
 */
TEST_F(TypeAnnotationTest, NamedTypeStatementType) {
    auto named_type = CreateNamedType("boolean");

    EXPECT_EQ(named_type->type(), StatementType::kNamedType);
}

/**
 * @test 测试NamedType的name()方法
 */
TEST_F(TypeAnnotationTest, NamedTypeNameMethod) {
    auto named_type1 = CreateNamedType("string");
    EXPECT_EQ(named_type1->name(), "string");

    auto named_type2 = CreateNamedType("number");
    EXPECT_EQ(named_type2->name(), "number");

    auto named_type3 = CreateNamedType("boolean");
    EXPECT_EQ(named_type3->name(), "boolean");
}

/**
 * @test 测试NamedType包含自定义类型名
 */
TEST_F(TypeAnnotationTest, NamedTypeCustomTypes) {
    auto named_type1 = CreateNamedType("MyCustomType");
    EXPECT_EQ(named_type1->name(), "MyCustomType");

    auto named_type2 = CreateNamedType("User");
    EXPECT_EQ(named_type2->name(), "User");

    auto named_type3 = CreateNamedType("ResponseType");
    EXPECT_EQ(named_type3->name(), "ResponseType");
}

/**
 * @test 测试NamedType泛型类型名
 */
TEST_F(TypeAnnotationTest, NamedTypeGenericTypes) {
    auto named_type1 = CreateNamedType("Array");
    EXPECT_EQ(named_type1->name(), "Array");

    auto named_type2 = CreateNamedType("Promise");
    EXPECT_EQ(named_type2->name(), "Promise");

    auto named_type3 = CreateNamedType("Map");
    EXPECT_EQ(named_type3->name(), "Map");
}

// ============================================================================
// 类型注解与命名类型组合测试
// ============================================================================

/**
 * @test 测试string类型注解
 */
TEST_F(TypeAnnotationTest, StringAnnotation) {
    auto named_type = CreateNamedType("string");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "string");
}

/**
 * @test 测试number类型注解
 */
TEST_F(TypeAnnotationTest, NumberAnnotation) {
    auto named_type = CreateNamedType("number");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "number");
}

/**
 * @test 测试boolean类型注解
 */
TEST_F(TypeAnnotationTest, BooleanAnnotation) {
    auto named_type = CreateNamedType("boolean");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "boolean");
}

/**
 * @test 测试any类型注解
 */
TEST_F(TypeAnnotationTest, AnyAnnotation) {
    auto named_type = CreateNamedType("any");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "any");
}

/**
 * @test 测试void类型注解
 */
TEST_F(TypeAnnotationTest, VoidAnnotation) {
    auto named_type = CreateNamedType("void");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "void");
}

/**
 * @test 测试自定义类型注解
 */
TEST_F(TypeAnnotationTest, CustomTypeAnnotation) {
    auto named_type = CreateNamedType("MyClass");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "MyClass");
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空类型名
 */
TEST_F(TypeAnnotationTest, EmptyTypeName) {
    auto named_type = CreateNamedType("");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_TRUE(type_ptr->name().empty());
}

/**
 * @test 测试包含数字的类型名
 */
TEST_F(TypeAnnotationTest, TypeNameWithNumbers) {
    auto named_type = CreateNamedType("Type123");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "Type123");
}

/**
 * @test 测试包含下划线的类型名
 */
TEST_F(TypeAnnotationTest, TypeNameWithUnderscores) {
    auto named_type = CreateNamedType("my_custom_type");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "my_custom_type");
}

// ============================================================================
// 移动语义测试
// ============================================================================

/**
 * @test 测试TypeAnnotation移动构造
 */
TEST_F(TypeAnnotationTest, MoveSemantics) {
    auto named_type = CreateNamedType("TestType");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    // 验证对象被正确移动
    ASSERT_NE(annotation, nullptr);
    ASSERT_NE(annotation->type_p(), nullptr);
}

// ============================================================================
// 代码生成接口测试
// ============================================================================

/**
 * @test 测试GenerateCode接口存在
 */
TEST_F(TypeAnnotationTest, CodeGenerationInterface) {
    auto named_type = CreateNamedType("TestType");
    auto annotation = CreateTypeAnnotation(std::move(named_type));

    // 验证对象可以被正确创建,且接口存在
    ASSERT_NE(annotation, nullptr);
    EXPECT_EQ(annotation->type(), StatementType::kTypeAnnotation);
    // 注意:实际的代码生成测试需要CodeGenerator和FunctionDef的完整设置
    // 类型注解在运行时不生成代码,只在编译时用于类型检查
}

// ============================================================================
// TryParseTypeAnnotation 方法测试
// ============================================================================

/**
 * @test 测试TryParseTypeAnnotation在没有冒号时返回nullptr
 */
TEST_F(TypeAnnotationTest, TryParseWithoutColon) {
    Lexer lexer("x");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    EXPECT_EQ(annotation, nullptr);
}

/**
 * @test 测试TryParseTypeAnnotation成功解析命名类型
 */
TEST_F(TypeAnnotationTest, TryParseNamedType) {
    Lexer lexer(": string");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    ASSERT_NE(annotation, nullptr);
    EXPECT_EQ(annotation->type(), StatementType::kTypeAnnotation);

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "string");
}

/**
 * @test 测试TryParseTypeAnnotation解析自定义命名类型
 */
TEST_F(TypeAnnotationTest, TryParseCustomNamedType) {
    Lexer lexer(": MyClass");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    ASSERT_NE(annotation, nullptr);
    EXPECT_EQ(annotation->type(), StatementType::kTypeAnnotation);

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "MyClass");
}

/**
 * @test 测试TryParseTypeAnnotation解析number类型
 */
TEST_F(TypeAnnotationTest, TryParseNumberType) {
    Lexer lexer(": number");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    ASSERT_NE(annotation, nullptr);

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "number");
}

/**
 * @test 测试TryParseTypeAnnotation解析boolean类型
 */
TEST_F(TypeAnnotationTest, TryParseBooleanType) {
    Lexer lexer(": boolean");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    ASSERT_NE(annotation, nullptr);

    auto* type_ptr = dynamic_cast<NamedType*>(annotation->type_p().get());
    ASSERT_NE(type_ptr, nullptr);
    EXPECT_EQ(type_ptr->name(), "boolean");
}

/**
 * @test 测试TryParseTypeAnnotation遇到左括号时抛出异常(当前实现未完成)
 */
TEST_F(TypeAnnotationTest, TryParseUnionTypeNotImplemented) {
    Lexer lexer(": (string|number)");

    // 当前实现检测到左括号时会调用ParseUnionType,
    // 但ParseUnionType期望identifier而不是左括号,所以会抛出异常
    EXPECT_THROW(
        TypeAnnotation::TryParseTypeAnnotation(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试TryParseTypeAnnotation遇到左括号时抛出异常(多个类型)
 */
TEST_F(TypeAnnotationTest, TryParseThreeWayUnionTypeNotImplemented) {
    Lexer lexer(": (string|number|boolean)");

    // 当前实现不支持带括号的联合类型语法
    EXPECT_THROW(
        TypeAnnotation::TryParseTypeAnnotation(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试TryParseTypeAnnotation在无效类型时抛出异常
 */
TEST_F(TypeAnnotationTest, TryParseInvalidType) {
    Lexer lexer(": @");

    EXPECT_THROW(
        TypeAnnotation::TryParseTypeAnnotation(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试TryParseTypeAnnotation在冒号后紧跟非法字符时抛出异常
 */
TEST_F(TypeAnnotationTest, TryParseInvalidTypeAfterColon) {
    Lexer lexer(":");

    EXPECT_THROW(
        TypeAnnotation::TryParseTypeAnnotation(&lexer),
        SyntaxError
    );
}

/**
 * @test 测试TryParseTypeAnnotation位置信息
 */
TEST_F(TypeAnnotationTest, TryParsePosition) {
    Lexer lexer(": string");
    auto annotation = TypeAnnotation::TryParseTypeAnnotation(&lexer);

    ASSERT_NE(annotation, nullptr);
    EXPECT_EQ(annotation->start(), 0);
    EXPECT_GT(annotation->end(), annotation->start());
}

} // namespace test
} // namespace compiler
} // namespace mjs

/**
 * @file declaration_statement_test.cpp
 * @brief 声明语句测试
 *
 * 测试声明语句类型，包括:
 * - 变量声明 (VariableDeclaration)
 * - 类声明 (ClassDeclaration)
 * - 函数声明
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/variable_declaration.h"
#include "src/compiler/statement_impl/class_declaration.h"
#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/statement_impl/expression_statement.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/function_expression.h"
#include "src/compiler/expression_impl/class_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class DeclarationStatementTest
 * @brief 声明语句测试类
 */
class DeclarationStatementTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：创建Parser对象
     * @param source 源代码字符串
     * @return Parser对象的唯一指针
     */
    std::unique_ptr<Parser> CreateParser(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return std::make_unique<Parser>(lexer.release());
    }

    /**
     * @brief 辅助方法：解析语句
     * @param source 源代码字符串
     * @return Statement对象的唯一指针
     */
    std::unique_ptr<Statement> ParseStatement(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return Statement::ParseStatement(lexer.get());
    }
};

// ============================================================================
// 变量声明测试 (VariableDeclaration)
// ============================================================================

/**
 * @test 测试var变量声明，编译器不支持Var
 */
//TEST_F(DeclarationStatementTest, VarDeclaration) {
//    auto stmt = ParseStatement("var x;");
//    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
//    ASSERT_NE(var_decl, nullptr);
//    EXPECT_EQ(var_decl->type(), StatementType::kVariableDeclaration);
//    EXPECT_EQ(var_decl->name(), "x");
//    EXPECT_EQ(var_decl->kind(), TokenType::kVar);
//    EXPECT_EQ(var_decl->init(), nullptr);
//}

/**
 * @test 测试let变量声明
 */
// TEST_F(DeclarationStatementTest, LetDeclaration) {
//     auto stmt = ParseStatement("let x;");
//     auto* let_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
//     ASSERT_NE(let_decl, nullptr);
//     EXPECT_EQ(let_decl->type(), StatementType::kVariableDeclaration);
//     EXPECT_EQ(let_decl->name(), "x");
//     EXPECT_EQ(let_decl->kind(), TokenType::kKwLet);
//     EXPECT_EQ(let_decl->init(), nullptr);
// }

/**
 * @test 测试const变量声明
 */
// TEST_F(DeclarationStatementTest, ConstDeclaration) {
//     auto stmt = ParseStatement("const x;");
//     auto* const_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
//     ASSERT_NE(const_decl, nullptr);
//     EXPECT_EQ(const_decl->type(), StatementType::kVariableDeclaration);
//     EXPECT_EQ(const_decl->name(), "x");
//     EXPECT_EQ(const_decl->kind(), TokenType::kKwConst);
//     EXPECT_EQ(const_decl->init(), nullptr);
// }

/**
 * @test 测试变量声明带初始化器
 */
// TEST_F(DeclarationStatementTest, VariableDeclarationWithInitializer) {
//     auto stmt = ParseStatement("var x = 42;");
//     auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
//     ASSERT_NE(var_decl, nullptr);
//     EXPECT_EQ(var_decl->name(), "x");
//     EXPECT_NE(var_decl->init(), nullptr);

//     auto* init_literal = dynamic_cast<IntegerLiteral*>(var_decl->init().get());
//     ASSERT_NE(init_literal, nullptr);
//     EXPECT_EQ(init_literal->value(), 42);
// }

/**
 * @test 测试let变量声明带初始化器
 */
TEST_F(DeclarationStatementTest, LetDeclarationWithInitializer) {
    auto stmt = ParseStatement("let y = 100;");
    auto* let_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(let_decl, nullptr);
    EXPECT_EQ(let_decl->name(), "y");
    EXPECT_EQ(let_decl->kind(), TokenType::kKwLet);
    EXPECT_NE(let_decl->init(), nullptr);
}

/**
 * @test 测试const变量声明带初始化器
 */
TEST_F(DeclarationStatementTest, ConstDeclarationWithInitializer) {
    auto stmt = ParseStatement("const z = 200;");
    auto* const_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(const_decl, nullptr);
    EXPECT_EQ(const_decl->name(), "z");
    EXPECT_EQ(const_decl->kind(), TokenType::kKwConst);
    EXPECT_NE(const_decl->init(), nullptr);
}

/**
 * @test 测试变量声明带字符串初始化器
 */
TEST_F(DeclarationStatementTest, VariableDeclarationWithStringInitializer) {
    auto stmt = ParseStatement("let name = 'hello';");
    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(var_decl, nullptr);
    EXPECT_NE(var_decl->init(), nullptr);

    auto* init_string = dynamic_cast<StringLiteral*>(var_decl->init().get());
    ASSERT_NE(init_string, nullptr);
    EXPECT_EQ(init_string->value(), "hello");
}

/**
 * @test 测试变量声明带标识符初始化器
 */
TEST_F(DeclarationStatementTest, VariableDeclarationWithIdentifierInitializer) {
    auto stmt = ParseStatement("let x = y;");
    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(var_decl, nullptr);
    EXPECT_NE(var_decl->init(), nullptr);

    auto* init_identifier = dynamic_cast<Identifier*>(var_decl->init().get());
    ASSERT_NE(init_identifier, nullptr);
    EXPECT_EQ(init_identifier->name(), "y");
}

/**
 * @test 测试变量声明带表达式初始化器
 */
TEST_F(DeclarationStatementTest, VariableDeclarationWithExpressionInitializer) {
    auto stmt = ParseStatement("let x = 1 + 2;");
    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(var_decl, nullptr);
    EXPECT_NE(var_decl->init(), nullptr);
}

/**
 * @test 测试变量声明带复杂表达式初始化器
 */
TEST_F(DeclarationStatementTest, VariableDeclarationWithComplexExpressionInitializer) {
    auto stmt = ParseStatement("let result = a * b + c / d;");
    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(var_decl, nullptr);
    EXPECT_NE(var_decl->init(), nullptr);
}

/**
 * @test 测试变量声明源代码位置
 */
// TEST_F(DeclarationStatementTest, VariableDeclarationSourcePosition) {
//     auto stmt = ParseStatement("var x;");
//     auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
//     ASSERT_NE(var_decl, nullptr);

//     auto start = var_decl->start();
//     auto end = var_decl->end();
//     // start位置验证
//     // start位置验证
//     // end位置验证
// }

/**
 * @test 测试多个变量声明
 */
TEST_F(DeclarationStatementTest, MultipleVariableDeclarations) {
    // auto stmt1 = ParseStatement("var x;");
    auto stmt2 = ParseStatement("let y;");
    auto stmt3 = ParseStatement("const z = 1;");

    // auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt1.get());
    auto* let_decl = dynamic_cast<VariableDeclaration*>(stmt2.get());
    auto* const_decl = dynamic_cast<VariableDeclaration*>(stmt3.get());

    // ASSERT_NE(var_decl, nullptr);
    ASSERT_NE(let_decl, nullptr);
    ASSERT_NE(const_decl, nullptr);

    // EXPECT_EQ(var_decl->kind(), TokenType::kKwVar);
    EXPECT_EQ(let_decl->kind(), TokenType::kKwLet);
    EXPECT_EQ(const_decl->kind(), TokenType::kKwConst);
}

/**
 * @test 测试变量声明在块语句中
 */
TEST_F(DeclarationStatementTest, VariableDeclarationInBlock) {
    auto stmt = ParseStatement("{ const x = 10; let y = 20; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->statements().size(), 2);

    const auto& first_stmt = block_stmt->statements()[0];
    auto* var_decl = dynamic_cast<VariableDeclaration*>(first_stmt.get());
    ASSERT_NE(var_decl, nullptr);
    EXPECT_EQ(var_decl->name(), "x");

    const auto& second_stmt = block_stmt->statements()[1];
    auto* let_decl = dynamic_cast<VariableDeclaration*>(second_stmt.get());
    ASSERT_NE(let_decl, nullptr);
    EXPECT_EQ(let_decl->name(), "y");
}

// ============================================================================
// 类声明测试 (ClassDeclaration)
// ============================================================================

/**
 * @test 测试简单类声明
 */
TEST_F(DeclarationStatementTest, SimpleClassDeclaration) {
    auto stmt = ParseStatement("class MyClass {}");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->type(), StatementType::kClassDeclaration);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_FALSE(class_decl->has_super_class());
}

/**
 * @test 测试类声明带构造函数
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithConstructor) {
    auto stmt = ParseStatement("class MyClass { constructor() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明带方法
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithMethod) {
    auto stmt = ParseStatement("class MyClass { myMethod() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明带多个方法
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithMultipleMethods) {
    auto stmt = ParseStatement("class MyClass { method1() {} method2() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 1);
}

/**
 * @test 测试类继承
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithExtends) {
    auto stmt = ParseStatement("class MyClass extends BaseClass {}");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_TRUE(class_decl->has_super_class());
    EXPECT_NE(class_decl->super_class(), nullptr);
}

/**
 * @test 测试类声明带静态方法
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithStaticMethod) {
    auto stmt = ParseStatement("class MyClass { static myStaticMethod() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明带getter
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithGetter) {
    auto stmt = ParseStatement("class MyClass { get myProperty() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明带setter
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithSetter) {
    auto stmt = ParseStatement("class MyClass { set myProperty(value) {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明带字段
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithField) {
    auto stmt = ParseStatement("class MyClass { myField = 42; }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 0);
}

/**
 * @test 测试类声明源代码位置
 */
TEST_F(DeclarationStatementTest, ClassDeclarationSourcePosition) {
    auto stmt = ParseStatement("class MyClass {}");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);

    auto start = class_decl->start();
    auto end = class_decl->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

/**
 * @test 测试嵌套类声明
 */
TEST_F(DeclarationStatementTest, NestedClassDeclaration) {
    auto stmt = ParseStatement("{ class Outer { class Inner {} } }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
}

/**
 * @test 测试类声明带计算属性名
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithComputedPropertyName) {
    auto stmt = ParseStatement("class MyClass { [methodName]() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
}

/**
 * @test 测试类声明带私有字段
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithPrivateField) {
    auto stmt = ParseStatement("class MyClass { #privateField = 42; }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
}

/**
 * @test 测试类声明带构造函数和方法
 */
TEST_F(DeclarationStatementTest, ClassDeclarationWithConstructorAndMethods) {
    auto stmt = ParseStatement("class MyClass { constructor(x) { this.x = x; } method1() {} method2() {} }");
    auto* class_decl = dynamic_cast<ClassDeclaration*>(stmt.get());
    ASSERT_NE(class_decl, nullptr);
    EXPECT_EQ(class_decl->id(), "MyClass");
    EXPECT_GT(class_decl->elements().size(), 1);
}

// ============================================================================
// 函数声明测试
// ============================================================================

/**
 * @test 测试简单函数声明
 */
TEST_F(DeclarationStatementTest, SimpleFunctionDeclaration) {
    auto stmt = ParseStatement("function myFunction() {}");
    // 函数声明可能被解析为FunctionExpression或其他类型
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试带参数的函数声明
 */
TEST_F(DeclarationStatementTest, FunctionDeclarationWithParameters) {
    auto stmt = ParseStatement("function myFunction(x, y) {}");
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试带默认参数的函数声明
 */
TEST_F(DeclarationStatementTest, FunctionDeclarationWithDefaultParameters) {
    auto stmt = ParseStatement("function myFunction(x = 1, y = 2) {}");
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试带剩余参数的函数声明
 */
TEST_F(DeclarationStatementTest, FunctionDeclarationWithRestParameter) {
    auto stmt = ParseStatement("function myFunction(...args) {}");
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试async函数声明
 */
TEST_F(DeclarationStatementTest, AsyncFunctionDeclaration) {
    auto stmt = ParseStatement("async function myAsyncFunction() {}");
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试generator函数声明
 */
TEST_F(DeclarationStatementTest, GeneratorFunctionDeclaration) {
    auto stmt = ParseStatement("function* myGeneratorFunction() {}");
    ASSERT_NE(stmt, nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

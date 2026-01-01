/**
 * @file identifier_primary_expression_test.cpp
 * @brief 标识符和主表达式测试
 *
 * 测试标识符和主表达式相关功能,包括:
 * - Identifier (标识符)
 * - PrimaryExpression (主表达式)
 * - ThisExpression (this表达式)
 * - SuperExpression (super表达式)
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/error.h>

#include "../src/compiler/lexer.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/expression.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/primary_expression.h"
#include "../src/compiler/expression_impl/this_expression.h"
#include "../src/compiler/expression_impl/super_expression.h"
#include "../src/compiler/expression_impl/integer_literal.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class IdentifierPrimaryExpressionTest
 * @brief 标识符和主表达式测试类
 */
class IdentifierPrimaryExpressionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：解析表达式
     * @param source 源代码字符串
     * @return Expression对象的唯一指针
     */
    std::unique_ptr<Expression> ParseExpression(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return Expression::ParseExpression(lexer.get());
    }
};

// ============================================================================
// 标识符测试
// ============================================================================

/**
 * @test 测试简单标识符
 */
TEST_F(IdentifierPrimaryExpressionTest, SimpleIdentifier) {
    // 测试基本标识符
    auto expr1 = ParseExpression("x");
    auto* ident1 = dynamic_cast<Identifier*>(expr1.get());
    ASSERT_NE(ident1, nullptr);
    EXPECT_EQ(ident1->name(), "x");

    // 测试下划线开头的标识符
    auto expr2 = ParseExpression("_private");
    auto* ident2 = dynamic_cast<Identifier*>(expr2.get());
    ASSERT_NE(ident2, nullptr);
    EXPECT_EQ(ident2->name(), "_private");

    // 测试包含数字的标识符
    auto expr3 = ParseExpression("value123");
    auto* ident3 = dynamic_cast<Identifier*>(expr3.get());
    ASSERT_NE(ident3, nullptr);
    EXPECT_EQ(ident3->name(), "value123");

    // 注意: $ 符号可能不被当前词法分析器支持
    // 如果需要支持,需要更新词法分析器
}

/**
 * @test 测试驼峰命名标识符
 */
TEST_F(IdentifierPrimaryExpressionTest, CamelCaseIdentifier) {
    // 小驼峰
    auto expr1 = ParseExpression("myVariable");
    auto* ident1 = dynamic_cast<Identifier*>(expr1.get());
    ASSERT_NE(ident1, nullptr);
    EXPECT_EQ(ident1->name(), "myVariable");

    // 大驼峰
    auto expr2 = ParseExpression("MyClass");
    auto* ident2 = dynamic_cast<Identifier*>(expr2.get());
    ASSERT_NE(ident2, nullptr);
    EXPECT_EQ(ident2->name(), "MyClass");
}

/**
 * @test 测试长标识符
 */
TEST_F(IdentifierPrimaryExpressionTest, LongIdentifier) {
    // 测试长标识符
    auto expr1 = ParseExpression("thisIsAVeryLongVariableName");
    auto* ident1 = dynamic_cast<Identifier*>(expr1.get());
    ASSERT_NE(ident1, nullptr);
    EXPECT_EQ(ident1->name(), "thisIsAVeryLongVariableName");
}

// ============================================================================
// 括号表达式测试
// ============================================================================

/**
 * @test 测试简单括号表达式
 */
TEST_F(IdentifierPrimaryExpressionTest, SimpleParenthesizedExpression) {
    // 括号包裹标识符
    auto expr1 = ParseExpression("(x)");
    auto* ident1 = dynamic_cast<Identifier*>(expr1.get());
    ASSERT_NE(ident1, nullptr);
    EXPECT_EQ(ident1->name(), "x");

    // 括号包裹数字
    auto expr2 = ParseExpression("(42)");
    auto* int_lit = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit, nullptr);
    EXPECT_EQ(int_lit->value(), 42);
}

/**
 * @test 测试嵌套括号表达式
 */
TEST_F(IdentifierPrimaryExpressionTest, NestedParenthesizedExpression) {
    // 多层嵌套括号
    auto expr1 = ParseExpression("((x))");
    auto* ident1 = dynamic_cast<Identifier*>(expr1.get());
    ASSERT_NE(ident1, nullptr);
    EXPECT_EQ(ident1->name(), "x");

    auto expr2 = ParseExpression("(((42)))");
    auto* int_lit = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit, nullptr);
    EXPECT_EQ(int_lit->value(), 42);
}

/**
 * @test 测试括号包裹复杂表达式
 */
TEST_F(IdentifierPrimaryExpressionTest, ComplexParenthesizedExpression) {
    // 括号改变优先级
    auto expr1 = ParseExpression("(a + b)");
    // 这应该被解析为二元表达式
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("((a + b) * c)");
    // 这应该被解析为嵌套的二元表达式
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// this表达式测试
// ============================================================================

/**
 * @test 测试this关键字
 */
TEST_F(IdentifierPrimaryExpressionTest, ThisExpression) {
    auto expr = ParseExpression("this");
    auto* this_expr = dynamic_cast<ThisExpression*>(expr.get());
    ASSERT_NE(this_expr, nullptr);
}

/**
 * @test 测试this在复杂表达式中
 */
TEST_F(IdentifierPrimaryExpressionTest, ThisInComplexExpression) {
    // this 作为成员访问的对象
    auto expr1 = ParseExpression("this.x");
    ASSERT_NE(expr1.get(), nullptr);

    // this 作为函数调用
    auto expr2 = ParseExpression("this.method()");
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// super表达式测试
// ============================================================================

/**
 * @test 测试super关键字
 */
TEST_F(IdentifierPrimaryExpressionTest, SuperExpression) {
    auto expr = ParseExpression("super");
    auto* super_expr = dynamic_cast<SuperExpression*>(expr.get());
    ASSERT_NE(super_expr, nullptr);
}

/**
 * @test 测试super在复杂表达式中
 */
TEST_F(IdentifierPrimaryExpressionTest, SuperInComplexExpression) {
    // super 作为方法调用
    auto expr1 = ParseExpression("super.method()");
    ASSERT_NE(expr1.get(), nullptr);

    // super 作为构造函数调用
    auto expr2 = ParseExpression("super()");
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// 组合表达式测试
// ============================================================================

/**
 * @test 测试标识符和字面量组合
 */
TEST_F(IdentifierPrimaryExpressionTest, IdentifierWithLiterals) {
    // 标识符和数字
    auto expr1 = ParseExpression("x + 42");
    ASSERT_NE(expr1.get(), nullptr);

    // 标识符和字符串
    auto expr2 = ParseExpression("name + 'test'");
    ASSERT_NE(expr2.get(), nullptr);

    // 多个标识符
    auto expr3 = ParseExpression("a + b + c");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试括号和标识符组合
 */
TEST_F(IdentifierPrimaryExpressionTest, ParenthesizedWithIdentifier) {
    // 括号改变运算顺序
    auto expr1 = ParseExpression("(a + b) * c");
    ASSERT_NE(expr1.get(), nullptr);

    // 括号包裹函数调用
    auto expr2 = ParseExpression("(func())");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试this和super在成员访问中
 */
TEST_F(IdentifierPrimaryExpressionTest, ThisAndSuperMemberAccess) {
    // this 的属性访问
    auto expr1 = ParseExpression("this.property");
    ASSERT_NE(expr1.get(), nullptr);

    // this 的方法调用
    auto expr2 = ParseExpression("this.method(arg)");
    ASSERT_NE(expr2.get(), nullptr);

    // super 的方法调用
    auto expr3 = ParseExpression("super.method(arg)");
    ASSERT_NE(expr3.get(), nullptr);

    // this 和 super 链式调用
    auto expr4 = ParseExpression("this.obj.method()");
    ASSERT_NE(expr4.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试特殊标识符字符
 */
TEST_F(IdentifierPrimaryExpressionTest, SpecialCharactersInIdentifier) {
    // Unicode 字符(假设支持)
    // auto expr1 = ParseExpression("变量");
    // ASSERT_NE(expr1.get(), nullptr);

    // 希腊字母
    // auto expr2 = ParseExpression("αβγ");
    // ASSERT_NE(expr2.get(), nullptr);

    // 这些测试依赖于编译器对Unicode标识符的支持
}

/**
 * @test 测试保留字不能作为标识符
 */
TEST_F(IdentifierPrimaryExpressionTest, ReservedWordsAsIdentifiers) {
    // 保留字不应该被解析为标识符
    // 这些应该被解析为关键字,而不是标识符

    // if/else/while 等是关键字,不是标识符
    // 尝试解析 "if" 应该抛出异常或返回非 Identifier 类型
    // 这里我们测试它会抛出异常
    EXPECT_THROW({
        ParseExpression("if");
    }, SyntaxError);

    // 测试其他保留字
    EXPECT_THROW({
        ParseExpression("while");
    }, SyntaxError);

    EXPECT_THROW({
        ParseExpression("return");
    }, SyntaxError);
}

/**
 * @test 测试空括号
 */
TEST_F(IdentifierPrimaryExpressionTest, EmptyParentheses) {
    // 空括号可能应该报错或解析为空表达式
    // auto expr = ParseExpression("()");
    // 根据实现,这可能抛出异常或返回特殊值
}

} // namespace test
} // namespace compiler
} // namespace mjs

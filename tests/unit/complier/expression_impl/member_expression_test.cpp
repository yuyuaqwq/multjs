/**
 * @file member_expression_test.cpp
 * @brief 成员访问表达式测试
 *
 * 测试成员访问表达式，包括:
 * - 点号访问 (obj.prop)
 * - 方括号访问 (obj["prop"])
 * - 可选链访问 (obj?.prop)
 * - 嵌套访问 (obj.a.b.c)
 * - 计算属性名 (obj[key])
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/error.h>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/expression.h"
#include "src/compiler/expression_impl/member_expression.h"
#include "src/compiler/expression_impl/identifier.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class MemberExpressionTest
 * @brief 成员访问表达式测试类
 */
class MemberExpressionTest : public ::testing::Test {
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
// 点号访问测试
// ============================================================================

/**
 * @test 测试简单的点号访问
 */
TEST_F(MemberExpressionTest, SimpleDotNotation) {
    auto expr = ParseExpression("obj.prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
    EXPECT_FALSE(member_expr->optional());

    // 验证对象部分
    auto* obj = dynamic_cast<Identifier*>(member_expr->object().get());
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), "obj");

    // 验证属性部分
    auto* prop = dynamic_cast<Identifier*>(member_expr->property().get());
    ASSERT_NE(prop, nullptr);
    EXPECT_EQ(prop->name(), "prop");
}

/**
 * @test 测试嵌套点号访问
 */
TEST_F(MemberExpressionTest, NestedDotNotation) {
    auto expr = ParseExpression("obj.a.b");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());

    // 外层是 obj.a.b，所以property是b
    auto* prop = dynamic_cast<Identifier*>(member_expr->property().get());
    ASSERT_NE(prop, nullptr);
    EXPECT_EQ(prop->name(), "b");
}

/**
 * @test 测试深层嵌套点号访问
 */
TEST_F(MemberExpressionTest, DeepNestedDotNotation) {
    auto expr = ParseExpression("obj.a.b.c.d");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);

    // 最外层访问的是d属性
    auto* prop = dynamic_cast<Identifier*>(member_expr->property().get());
    ASSERT_NE(prop, nullptr);
    EXPECT_EQ(prop->name(), "d");
}

// ============================================================================
// 方括号访问测试
// ============================================================================

/**
 * @test 测试简单的方括号访问
 */
TEST_F(MemberExpressionTest, SimpleBracketNotation) {
    auto expr = ParseExpression("obj[prop]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
    EXPECT_FALSE(member_expr->optional());

    // 验证对象部分
    auto* obj = dynamic_cast<Identifier*>(member_expr->object().get());
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(obj->name(), "obj");
}

/**
 * @test 测试方括号中使用字符串字面量
 */
TEST_F(MemberExpressionTest, BracketNotationWithStringLiteral) {
    auto expr = ParseExpression("obj[\"prop\"]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
}

/**
 * @test 测试方括号中使用数字
 */
TEST_F(MemberExpressionTest, BracketNotationWithNumber) {
    auto expr = ParseExpression("arr[0]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
}

/**
 * @test 测试嵌套方括号访问
 */
TEST_F(MemberExpressionTest, NestedBracketNotation) {
    auto expr = ParseExpression("obj[arr[0]]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
}

// ============================================================================
// 可选链访问测试
// ============================================================================

/**
 * @test 测试可选链点号访问
 */
TEST_F(MemberExpressionTest, OptionalChainingWithDot) {
    auto expr = ParseExpression("obj?.prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
    EXPECT_TRUE(member_expr->optional());
}

/**
 * @test 测试可选链方括号访问
 */
TEST_F(MemberExpressionTest, OptionalChainingWithBracket) {
    auto expr = ParseExpression("obj?.[prop]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
    EXPECT_TRUE(member_expr->optional());
}

/**
 * @test 测试嵌套可选链
 */
TEST_F(MemberExpressionTest, NestedOptionalChaining) {
    auto expr = ParseExpression("obj?.a?.b");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->optional());
}

// ============================================================================
// 混合访问测试
// ============================================================================

/**
 * @test 测试点号和方括号混合访问
 */
TEST_F(MemberExpressionTest, MixedDotAndBracketNotation) {
    auto expr = ParseExpression("obj.a[0]");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_TRUE(member_expr->computed());
}

/**
 * @test 测试方括号和点号混合访问
 */
TEST_F(MemberExpressionTest, MixedBracketAndDotNotation) {
    auto expr = ParseExpression("obj[0].a");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
}

/**
 * @test 测试复杂混合访问
 */
TEST_F(MemberExpressionTest, ComplexMixedAccess) {
    auto expr1 = ParseExpression("obj.a[0].b");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("obj[0].a[1]");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("obj?.a[0]?.b");
    ASSERT_NE(expr3.get(), nullptr);
}

// ============================================================================
// 方法调用相关测试
// ============================================================================

/**
 * @test 测试方法访问
 */
TEST_F(MemberExpressionTest, MethodAccess) {
    auto expr = ParseExpression("obj.method");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->is_method_call()); // 只是访问，不是调用
}

/**
 * @test 测试方法调用
 */
TEST_F(MemberExpressionTest, MethodCall) {
    auto expr = ParseExpression("obj.method()");
    // 这是CallExpression，不是MemberExpression
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试带参数的方法调用
 */
TEST_F(MemberExpressionTest, MethodCallWithArguments) {
    auto expr = ParseExpression("obj.method(a, b, c)");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试this的属性访问
 */
TEST_F(MemberExpressionTest, ThisPropertyAccess) {
    auto expr = ParseExpression("this.prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
}

/**
 * @test 测试super的属性访问
 */
TEST_F(MemberExpressionTest, SuperPropertyAccess) {
    auto expr = ParseExpression("super.prop");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试函数返回值的属性访问
 */
TEST_F(MemberExpressionTest, FunctionReturnPropertyAccess) {
    auto expr = ParseExpression("func().prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
}

/**
 * @test 测试表达式的属性访问
 */
TEST_F(MemberExpressionTest, ExpressionPropertyAccess) {
    auto expr1 = ParseExpression("(a + b).prop");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("a || b.prop");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试连续点号访问
 */
TEST_F(MemberExpressionTest, ConsecutiveDotAccess) {
    auto expr = ParseExpression("a.b.c.d.e.f.g");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试连续方括号访问
 */
TEST_F(MemberExpressionTest, ConsecutiveBracketAccess) {
    auto expr = ParseExpression("a[0][1][2]");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试计算属性表达式
 */
TEST_F(MemberExpressionTest, ComputedPropertyWithExpression) {
    auto expr1 = ParseExpression("obj[x + y]");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("obj[func()]");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("obj[arr[0]]");
    ASSERT_NE(expr3.get(), nullptr);
}

// ============================================================================
// 错误情况测试
// ============================================================================

/**
 * @test 测试空对象访问
 */
TEST_F(MemberExpressionTest, EmptyObjectAccess) {
    // 空对象后跟点号应该抛出错误
    EXPECT_THROW({
        ParseExpression(".prop");
    }, SyntaxError);
}

/**
 * @test 测试不匹配的括号
 */
TEST_F(MemberExpressionTest, UnmatchedBrackets) {
    EXPECT_THROW({
        ParseExpression("obj[0");
    }, SyntaxError);
}

} // namespace test
} // namespace compiler
} // namespace mjs

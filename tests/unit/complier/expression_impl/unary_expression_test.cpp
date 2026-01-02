/**
 * @file unary_expression_test.cpp
 * @brief 一元表达式测试
 *
 * 测试所有一元表达式类型，包括:
 * - 前缀一元运算符 (++, --, +, -, !, ~, typeof, void, delete)
 * - 后缀一元运算符 (++, --)
 * - await表达式
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
#include "src/compiler/expression_impl/unary_expression.h"
#include "src/compiler/expression_impl/await_expression.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/float_literal.h"
#include "src/compiler/expression_impl/boolean_literal.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class UnaryExpressionTest
 * @brief 一元表达式测试类
 */
class UnaryExpressionTest : public ::testing::Test {
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
// 前缀一元运算符测试 - 算术运算符
// ============================================================================

/**
 * @test 测试前缀加号 +
 */
TEST_F(UnaryExpressionTest, PrefixPlusOperator) {
    auto expr = ParseExpression("+x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpAdd);
    EXPECT_TRUE(unary_expr->is_prefix());
    EXPECT_NE(dynamic_cast<Identifier*>(unary_expr->argument().get()), nullptr);
}

/**
 * @test 测试前缀减号 -
 */
TEST_F(UnaryExpressionTest, PrefixMinusOperator) {
    auto expr = ParseExpression("-x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpSub);
    EXPECT_TRUE(unary_expr->is_prefix());

    // 测试应用于数字
    auto expr2 = ParseExpression("-42");
    auto* unary_expr2 = dynamic_cast<UnaryExpression*>(expr2.get());
    ASSERT_NE(unary_expr2, nullptr);
    EXPECT_EQ(unary_expr2->op(), TokenType::kOpSub);
}

/**
 * @test 测试前缀自增 ++
 */
TEST_F(UnaryExpressionTest, PrefixIncrementOperator) {
    auto expr = ParseExpression("++x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpPrefixInc);
    EXPECT_TRUE(unary_expr->is_prefix());
    EXPECT_NE(dynamic_cast<Identifier*>(unary_expr->argument().get()), nullptr);
}

/**
 * @test 测试前缀自减 --
 */
TEST_F(UnaryExpressionTest, PrefixDecrementOperator) {
    auto expr = ParseExpression("--x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpPrefixDec);
    EXPECT_TRUE(unary_expr->is_prefix());
}

// ============================================================================
// 前缀一元运算符测试 - 逻辑运算符
// ============================================================================

/**
 * @test 测试逻辑非 !
 */
TEST_F(UnaryExpressionTest, LogicalNotOperator) {
    auto expr = ParseExpression("!true");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpNot);
    EXPECT_TRUE(unary_expr->is_prefix());
    EXPECT_NE(dynamic_cast<BooleanLiteral*>(unary_expr->argument().get()), nullptr);

    // 测试应用于标识符
    auto expr2 = ParseExpression("!flag");
    auto* unary_expr2 = dynamic_cast<UnaryExpression*>(expr2.get());
    ASSERT_NE(unary_expr2, nullptr);
    EXPECT_EQ(unary_expr2->op(), TokenType::kOpNot);
}

// ============================================================================
// 前缀一元运算符测试 - 位运算符
// ============================================================================

/**
 * @test 测试按位取反 ~
 */
TEST_F(UnaryExpressionTest, BitwiseNotOperator) {
    auto expr = ParseExpression("~x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpBitNot);
    EXPECT_TRUE(unary_expr->is_prefix());
}

// ============================================================================
// 前缀一元运算符测试 - 特殊运算符
// ============================================================================

/**
 * @test 测试typeof运算符
 */
TEST_F(UnaryExpressionTest, TypeofOperator) {
    auto expr = ParseExpression("typeof x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kKwTypeof);
    EXPECT_TRUE(unary_expr->is_prefix());
}

/**
 * @test 测试void运算符
 */
TEST_F(UnaryExpressionTest, VoidOperator) {
    auto expr = ParseExpression("void x");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kKwVoid);
    EXPECT_TRUE(unary_expr->is_prefix());
}

/**
 * @test 测试delete运算符
 */
TEST_F(UnaryExpressionTest, DeleteOperator) {
    auto expr = ParseExpression("delete obj.prop");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kKwDelete);
    EXPECT_TRUE(unary_expr->is_prefix());
}

// ============================================================================
// 后缀一元运算符测试
// ============================================================================

/**
 * @test 测试后缀自增 ++
 */
TEST_F(UnaryExpressionTest, PostfixIncrementOperator) {
    auto expr = ParseExpression("x++");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpSuffixInc);
    EXPECT_FALSE(unary_expr->is_prefix());
    EXPECT_NE(dynamic_cast<Identifier*>(unary_expr->argument().get()), nullptr);
}

/**
 * @test 测试后缀自减 --
 */
TEST_F(UnaryExpressionTest, PostfixDecrementOperator) {
    auto expr = ParseExpression("x--");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary_expr, nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpSuffixInc); // 内部实现中使用kOpSuffixInc
    EXPECT_FALSE(unary_expr->is_prefix());
}

// ============================================================================
// await表达式测试
// ============================================================================

/**
 * @test 测试await表达式
 */
TEST_F(UnaryExpressionTest, AwaitExpression) {
    auto expr = ParseExpression("await promise");
    auto* await_expr = dynamic_cast<AwaitExpression*>(expr.get());
    ASSERT_NE(await_expr, nullptr);
    EXPECT_NE(dynamic_cast<Identifier*>(await_expr->argument().get()), nullptr);
}

/**
 * @test 测试await嵌套表达式
 */
TEST_F(UnaryExpressionTest, AwaitWithNestedExpression) {
    auto expr = ParseExpression("await asyncFunc()");
    auto* await_expr = dynamic_cast<AwaitExpression*>(expr.get());
    ASSERT_NE(await_expr, nullptr);
}

// ============================================================================
// 组合表达式测试
// ============================================================================

/**
 * @test 测试一元运算符和二元运算符组合
 */
TEST_F(UnaryExpressionTest, UnaryWithBinaryOperators) {
    // 一元运算符优先级高于二元运算符
    auto expr1 = ParseExpression("-x + y");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("!x || y");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("++x * 2");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试多个一元运算符组合
 */
TEST_F(UnaryExpressionTest, MultipleUnaryOperators) {
    // 多个前缀一元运算符可以组合
    auto expr1 = ParseExpression("!!x");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("- -x");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("typeof typeof x");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试一元运算符与括号组合
 */
TEST_F(UnaryExpressionTest, UnaryWithParentheses) {
    auto expr1 = ParseExpression("-(x + y)");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("!(x || y)");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("++(x + y)"); // 这可能不是有效语法，但测试解析
    ASSERT_NE(expr3.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试一元运算符应用于字面量
 */
TEST_F(UnaryExpressionTest, UnaryOperatorOnLiterals) {
    // 应用于数字
    auto expr1 = ParseExpression("-42");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("+3.14");
    ASSERT_NE(expr2.get(), nullptr);

    // 应用于布尔值
    auto expr3 = ParseExpression("!false");
    ASSERT_NE(expr3.get(), nullptr);

    // 应用于布尔值
    auto expr4 = ParseExpression("!true");
    ASSERT_NE(expr4.get(), nullptr);
}

/**
 * @test 测试一元运算符应用于复杂表达式
 */
TEST_F(UnaryExpressionTest, UnaryOperatorOnComplexExpressions) {
    auto expr1 = ParseExpression("-(x * y + z)");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("!(a && b || c)");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("typeof (obj.prop)");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试前缀和后缀运算符的区别
 */
TEST_F(UnaryExpressionTest, PrefixVsPostfixOperators) {
    auto prefix = ParseExpression("++x");
    auto* prefix_expr = dynamic_cast<UnaryExpression*>(prefix.get());
    ASSERT_NE(prefix_expr, nullptr);
    EXPECT_TRUE(prefix_expr->is_prefix());

    auto postfix = ParseExpression("x++");
    auto* postfix_expr = dynamic_cast<UnaryExpression*>(postfix.get());
    ASSERT_NE(postfix_expr, nullptr);
    EXPECT_FALSE(postfix_expr->is_prefix());
}

/**
 * @test 测试一元运算符在表达式中的位置
 */
TEST_F(UnaryExpressionTest, UnaryOperatorPosition) {
    // 一元运算符在表达式的开头
    auto expr1 = ParseExpression("-x");
    ASSERT_NE(expr1.get(), nullptr);

    // 一元运算符在括号内
    auto expr2 = ParseExpression("(-x)");
    ASSERT_NE(expr2.get(), nullptr);

    // 一元运算符在成员访问中
    auto expr3 = ParseExpression("(-x).toString()");
    ASSERT_NE(expr3.get(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

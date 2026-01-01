/**
 * @file binary_expression_test.cpp
 * @brief 二元表达式测试
 *
 * 测试所有二元表达式类型，包括:
 * - 算术运算符 (+, -, *, /, %, **)
 * - 比较运算符 (==, !=, ===, !==, <, >, <=, >=)
 * - 逻辑运算符 (&&, ||, ??)
 * - 位运算符 (&, |, ^, <<, >>, >>>)
 * - 运算符优先级
 * - 运算符结合性
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
#include "../src/compiler/expression_impl/binary_expression.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/integer_literal.h"
#include "../src/compiler/expression_impl/float_literal.h"
#include "../src/compiler/expression_impl/boolean_literal.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class BinaryExpressionTest
 * @brief 二元表达式测试类
 */
class BinaryExpressionTest : public ::testing::Test {
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
// 算术运算符测试
// ============================================================================

/**
 * @test 测试加法运算符 +
 */
TEST_F(BinaryExpressionTest, AdditionOperator) {
    auto expr = ParseExpression("a + b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpAdd);
    EXPECT_NE(dynamic_cast<Identifier*>(binary_expr->left().get()), nullptr);
    EXPECT_NE(dynamic_cast<Identifier*>(binary_expr->right().get()), nullptr);
}

/**
 * @test 测试减法运算符 -
 */
TEST_F(BinaryExpressionTest, SubtractionOperator) {
    auto expr = ParseExpression("a - b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpSub);
}

/**
 * @test 测试乘法运算符 *
 */
TEST_F(BinaryExpressionTest, MultiplicationOperator) {
    auto expr = ParseExpression("a * b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpMul);
}

/**
 * @test 测试除法运算符 /
 */
TEST_F(BinaryExpressionTest, DivisionOperator) {
    auto expr = ParseExpression("a / b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpDiv);
}

/**
 * @test 测试取模运算符 %
 */
TEST_F(BinaryExpressionTest, ModuloOperator) {
    auto expr = ParseExpression("a % b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpMod);
}

/**
 * @test 测试幂运算符 **
 */
TEST_F(BinaryExpressionTest, ExponentiationOperator) {
    auto expr = ParseExpression("a ** b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpPower);
}

// ============================================================================
// 比较运算符测试
// ============================================================================

/**
 * @test 测试相等运算符 ==
 */
TEST_F(BinaryExpressionTest, EqualityOperator) {
    auto expr = ParseExpression("a == b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpEq);
}

/**
 * @test 测试不等运算符 !=
 */
TEST_F(BinaryExpressionTest, InequalityOperator) {
    auto expr = ParseExpression("a != b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpNe);
}

/**
 * @test 测试严格相等运算符 ===
 */
TEST_F(BinaryExpressionTest, StrictEqualityOperator) {
    auto expr = ParseExpression("a === b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpStrictEq);
}

/**
 * @test 测试严格不等运算符 !==
 */
TEST_F(BinaryExpressionTest, StrictInequalityOperator) {
    auto expr = ParseExpression("a !== b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpStrictNe);
}

/**
 * @test 测试小于运算符 <
 */
TEST_F(BinaryExpressionTest, LessThanOperator) {
    auto expr = ParseExpression("a < b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpLt);
}

/**
 * @test 测试大于运算符 >
 */
TEST_F(BinaryExpressionTest, GreaterThanOperator) {
    auto expr = ParseExpression("a > b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpGt);
}

/**
 * @test 测试小于等于运算符 <=
 */
TEST_F(BinaryExpressionTest, LessThanOrEqualOperator) {
    auto expr = ParseExpression("a <= b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpLe);
}

/**
 * @test 测试大于等于运算符 >=
 */
TEST_F(BinaryExpressionTest, GreaterThanOrEqualOperator) {
    auto expr = ParseExpression("a >= b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpGe);
}

// ============================================================================
// 逻辑运算符测试
// ============================================================================

/**
 * @test 测试逻辑与运算符 &&
 */
TEST_F(BinaryExpressionTest, LogicalAndOperator) {
    auto expr = ParseExpression("a && b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpAnd);
}

/**
 * @test 测试逻辑或运算符 ||
 */
TEST_F(BinaryExpressionTest, LogicalOrOperator) {
    auto expr = ParseExpression("a || b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpOr);
}

/**
 * @test 测试空值合并运算符 ??
 */
TEST_F(BinaryExpressionTest, NullishCoalescingOperator) {
    auto expr = ParseExpression("a ?? b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpNullishCoalescing);
}

// ============================================================================
// 位运算符测试
// ============================================================================

/**
 * @test 测试按位与运算符 &
 */
TEST_F(BinaryExpressionTest, BitwiseAndOperator) {
    auto expr = ParseExpression("a & b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpBitAnd);
}

/**
 * @test 测试按位或运算符 |
 */
TEST_F(BinaryExpressionTest, BitwiseOrOperator) {
    auto expr = ParseExpression("a | b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpBitOr);
}

/**
 * @test 测试按位异或运算符 ^
 */
TEST_F(BinaryExpressionTest, BitwiseXorOperator) {
    auto expr = ParseExpression("a ^ b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpBitXor);
}

/**
 * @test 测试左移运算符 <<
 */
TEST_F(BinaryExpressionTest, LeftShiftOperator) {
    auto expr = ParseExpression("a << b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpShiftLeft);
}

/**
 * @test 测试右移运算符 >>
 */
TEST_F(BinaryExpressionTest, RightShiftOperator) {
    auto expr = ParseExpression("a >> b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpShiftRight);
}

/**
 * @test 测试无符号右移运算符 >>>
 */
TEST_F(BinaryExpressionTest, UnsignedRightShiftOperator) {
    auto expr = ParseExpression("a >>> b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpUnsignedShiftRight);
}

// ============================================================================
// 逗号运算符测试
// ============================================================================

/**
 * @test 测试逗号运算符
 */
TEST_F(BinaryExpressionTest, CommaOperator) {
    auto expr = ParseExpression("a, b");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kSepComma);
}

// ============================================================================
// 运算符优先级测试
// ============================================================================

/**
 * @test 测试乘法优先级高于加法
 */
TEST_F(BinaryExpressionTest, MultiplicationHigherThanAddition) {
    auto expr = ParseExpression("a + b * c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpAdd);

    // 右侧应该是乘法表达式
    auto* right_expr = dynamic_cast<BinaryExpression*>(binary_expr->right().get());
    ASSERT_NE(right_expr, nullptr);
    EXPECT_EQ(right_expr->op(), TokenType::kOpMul);
}

/**
 * @test 测试括号改变优先级
 */
TEST_F(BinaryExpressionTest, ParenthesesChangePrecedence) {
    auto expr = ParseExpression("(a + b) * c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpMul);

    // 左侧应该是加法表达式（被括号包裹）
    auto* left_expr = dynamic_cast<BinaryExpression*>(binary_expr->left().get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->op(), TokenType::kOpAdd);
}

/**
 * @test 测试幂运算符优先级
 */
TEST_F(BinaryExpressionTest, ExponentiationPrecedence) {
    auto expr = ParseExpression("a ** b ** c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpPower);

    // 幂运算符是右结合的，所以右侧应该是另一个幂运算
    auto* right_expr = dynamic_cast<BinaryExpression*>(binary_expr->right().get());
    ASSERT_NE(right_expr, nullptr);
    EXPECT_EQ(right_expr->op(), TokenType::kOpPower);
}

/**
 * @test 测试比较运算符优先级低于算术运算符
 */
TEST_F(BinaryExpressionTest, ComparisonLowerThanArithmetic) {
    auto expr = ParseExpression("a + b < c * d");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpLt);

    // 左右两侧都应该是算术表达式
    auto* left_expr = dynamic_cast<BinaryExpression*>(binary_expr->left().get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->op(), TokenType::kOpAdd);
}

/**
 * @test 测试逻辑与优先级高于逻辑或
 */
TEST_F(BinaryExpressionTest, LogicalAndHigherThanLogicalOr) {
    auto expr = ParseExpression("a || b && c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpOr);

    // 右侧应该是逻辑与表达式
    auto* right_expr = dynamic_cast<BinaryExpression*>(binary_expr->right().get());
    ASSERT_NE(right_expr, nullptr);
    EXPECT_EQ(right_expr->op(), TokenType::kOpAnd);
}

/**
 * @test 测试位运算符优先级
 */
TEST_F(BinaryExpressionTest, BitwiseOperatorPrecedence) {
    auto expr = ParseExpression("a & b | c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpBitOr);

    // 左侧应该是按位与表达式
    auto* left_expr = dynamic_cast<BinaryExpression*>(binary_expr->left().get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->op(), TokenType::kOpBitAnd);
}

// ============================================================================
// 运算符结合性测试
// ============================================================================

/**
 * @test 测试加法运算符的左结合性
 */
TEST_F(BinaryExpressionTest, AdditionIsLeftAssociative) {
    auto expr = ParseExpression("a - b - c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpSub);

    // 左侧应该是另一个减法表达式（左结合）
    auto* left_expr = dynamic_cast<BinaryExpression*>(binary_expr->left().get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->op(), TokenType::kOpSub);
}

/**
 * @test 测试乘法运算符的左结合性
 */
TEST_F(BinaryExpressionTest, MultiplicationIsLeftAssociative) {
    auto expr = ParseExpression("a * b * c");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpMul);

    // 左侧应该是另一个乘法表达式
    auto* left_expr = dynamic_cast<BinaryExpression*>(binary_expr->left().get());
    ASSERT_NE(left_expr, nullptr);
    EXPECT_EQ(left_expr->op(), TokenType::kOpMul);
}

// ============================================================================
// 复杂表达式测试
// ============================================================================

/**
 * @test 测试多个运算符组合
 */
TEST_F(BinaryExpressionTest, MultipleOperatorsCombination) {
    auto expr = ParseExpression("a + b * c - d / e");
    ASSERT_NE(expr.get(), nullptr);

    auto expr2 = ParseExpression("a < b && c > d || e == f");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("a << 2 | b & c");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试运算符与字面量组合
 */
TEST_F(BinaryExpressionTest, OperatorsWithLiterals) {
    auto expr1 = ParseExpression("1 + 2");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("3.14 * 2");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("true && false");
    ASSERT_NE(expr3.get(), nullptr);

    auto expr4 = ParseExpression("'hello' + 'world'");
    ASSERT_NE(expr4.get(), nullptr);
}

/**
 * @test 测试嵌套表达式
 */
TEST_F(BinaryExpressionTest, NestedExpressions) {
    auto expr1 = ParseExpression("((a + b) * (c - d))");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("a && (b || c) && d");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("(a < b) == (c > d)");
    ASSERT_NE(expr3.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试连续的相同运算符
 */
TEST_F(BinaryExpressionTest, ConsecutiveSameOperators) {
    auto expr1 = ParseExpression("a + b + c + d");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("a && b && c && d");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试运算符与一元运算符组合
 */
TEST_F(BinaryExpressionTest, BinaryWithUnaryOperators) {
    auto expr1 = ParseExpression("-a + b");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("!a || b");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("++a * b");
    ASSERT_NE(expr3.get(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

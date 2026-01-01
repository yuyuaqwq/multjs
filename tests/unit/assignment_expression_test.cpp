/**
 * @file assignment_expression_test.cpp
 * @brief 赋值表达式测试
 *
 * 测试所有赋值表达式类型，包括:
 * - 简单赋值 (=)
 * - 复合赋值 (+=, -=, *=, /=, %=, **=, etc.)
 * - 位运算复合赋值 (&=, |=, ^=, <<=, >>=, >>>=)
 * - 链式赋值
 * - 解构赋值
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
#include "../src/compiler/expression_impl/assignment_expression.h"
#include "../src/compiler/expression_impl/binary_expression.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/integer_literal.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class AssignmentExpressionTest
 * @brief 赋值表达式测试类
 */
class AssignmentExpressionTest : public ::testing::Test {
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
// 简单赋值测试
// ============================================================================

/**
 * @test 测试简单赋值运算符 =
 */
TEST_F(AssignmentExpressionTest, SimpleAssignment) {
    auto expr = ParseExpression("x = 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);

    // 验证左值
    auto* left = dynamic_cast<Identifier*>(assign_expr->left().get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->name(), "x");

    // 验证右值
    auto* right = dynamic_cast<IntegerLiteral*>(assign_expr->right().get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value(), 5);
}

/**
 * @test 测试标识符赋值
 */
TEST_F(AssignmentExpressionTest, IdentifierAssignment) {
    auto expr = ParseExpression("a = b");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);

    auto* left = dynamic_cast<Identifier*>(assign_expr->left().get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->name(), "a");

    auto* right = dynamic_cast<Identifier*>(assign_expr->right().get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->name(), "b");
}

// ============================================================================
// 算术复合赋值测试
// ============================================================================

/**
 * @test 测试加法赋值 +=
 */
TEST_F(AssignmentExpressionTest, AddAssign) {
    auto expr = ParseExpression("x += 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAddAssign);
}

/**
 * @test 测试减法赋值 -=
 */
TEST_F(AssignmentExpressionTest, SubAssign) {
    auto expr = ParseExpression("x -= 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpSubAssign);
}

/**
 * @test 测试乘法赋值 *=
 */
TEST_F(AssignmentExpressionTest, MulAssign) {
    auto expr = ParseExpression("x *= 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpMulAssign);
}

/**
 * @test 测试除法赋值 /=
 */
TEST_F(AssignmentExpressionTest, DivAssign) {
    auto expr = ParseExpression("x /= 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpDivAssign);
}

/**
 * @test 测试取模赋值 %=
 */
TEST_F(AssignmentExpressionTest, ModAssign) {
    auto expr = ParseExpression("x %= 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpModAssign);
}

/**
 * @test 测试幂运算赋值 **=
 */
TEST_F(AssignmentExpressionTest, PowerAssign) {
    auto expr = ParseExpression("x **= 2");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpPowerAssign);
}

// ============================================================================
// 位运算复合赋值测试
// ============================================================================

/**
 * @test 测试按位与赋值 &=
 */
TEST_F(AssignmentExpressionTest, BitAndAssign) {
    auto expr = ParseExpression("x &= 0xFF");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpBitAndAssign);
}

/**
 * @test 测试按位或赋值 |=
 */
TEST_F(AssignmentExpressionTest, BitOrAssign) {
    auto expr = ParseExpression("x |= 0xFF");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpBitOrAssign);
}

/**
 * @test 测试按位异或赋值 ^=
 */
TEST_F(AssignmentExpressionTest, BitXorAssign) {
    auto expr = ParseExpression("x ^= 0xFF");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpBitXorAssign);
}

/**
 * @test 测试左移赋值 <<=
 */
TEST_F(AssignmentExpressionTest, LeftShiftAssign) {
    auto expr = ParseExpression("x <<= 2");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpShiftLeftAssign);
}

/**
 * @test 测试右移赋值 >>=
 */
TEST_F(AssignmentExpressionTest, RightShiftAssign) {
    auto expr = ParseExpression("x >>= 2");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpShiftRightAssign);
}

/**
 * @test 测试无符号右移赋值 >>>=
 */
TEST_F(AssignmentExpressionTest, UnsignedRightShiftAssign) {
    auto expr = ParseExpression("x >>>= 2");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpUnsignedShiftRightAssign);
}

// ============================================================================
// 链式赋值测试
// ============================================================================

/**
 * @test 测试简单链式赋值
 */
TEST_F(AssignmentExpressionTest, SimpleChainedAssignment) {
    auto expr = ParseExpression("a = b = 5");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);

    // 右侧应该是另一个赋值表达式
    auto* right_assign = dynamic_cast<AssignmentExpression*>(assign_expr->right().get());
    ASSERT_NE(right_assign, nullptr);
    EXPECT_EQ(right_assign->op(), TokenType::kOpAssign);
}

/**
 * @test 测试多个变量的链式赋值
 */
TEST_F(AssignmentExpressionTest, MultipleChainedAssignment) {
    auto expr = ParseExpression("a = b = c = d = 10");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);

    // 验证链式结构
    auto* right1 = dynamic_cast<AssignmentExpression*>(assign_expr->right().get());
    ASSERT_NE(right1, nullptr);

    auto* right2 = dynamic_cast<AssignmentExpression*>(right1->right().get());
    ASSERT_NE(right2, nullptr);

    auto* right3 = dynamic_cast<AssignmentExpression*>(right2->right().get());
    ASSERT_NE(right3, nullptr);
}

// ============================================================================
// 复合赋值与表达式组合测试
// ============================================================================

/**
 * @test 测试复合赋值右侧为表达式
 */
TEST_F(AssignmentExpressionTest, CompoundAssignWithExpression) {
    auto expr = ParseExpression("x += a + b");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAddAssign);

    // 右侧应该是一个二元表达式
    auto* right = dynamic_cast<BinaryExpression*>(assign_expr->right().get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->op(), TokenType::kOpAdd);
}

/**
 * @test 测试复合赋值右侧为函数调用
 */
TEST_F(AssignmentExpressionTest, CompoundAssignWithFunctionCall) {
    auto expr = ParseExpression("x += getValue()");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAddAssign);
    ASSERT_NE(assign_expr->right().get(), nullptr);
}

// ============================================================================
// 赋值与其他运算符优先级测试
// ============================================================================

/**
 * @test 测试赋值运算符优先级低于逗号运算符
 */
TEST_F(AssignmentExpressionTest, AssignmentLowerThanComma) {
    auto expr = ParseExpression("a = 1, b = 2");
    ASSERT_NE(expr.get(), nullptr);
    // 逗号运算符应该先结合，所以整体应该是一个逗号表达式
    auto* comma_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(comma_expr, nullptr);
    EXPECT_EQ(comma_expr->op(), TokenType::kSepComma);
}

/**
 * @test 测试赋值运算符右结合性
 */
TEST_F(AssignmentExpressionTest, AssignmentIsRightAssociative) {
    auto expr = ParseExpression("a = b = c");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_NE(assign_expr, nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);

    // 由于右结合性，右侧应该是另一个赋值表达式
    auto* right_assign = dynamic_cast<AssignmentExpression*>(assign_expr->right().get());
    ASSERT_NE(right_assign, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试赋值为复杂表达式
 */
TEST_F(AssignmentExpressionTest, AssignmentWithComplexExpression) {
    auto expr1 = ParseExpression("x = (a + b) * c");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("y = a && b || c");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("z = a < b ? c : d");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试赋值与一元运算符组合
 */
TEST_F(AssignmentExpressionTest, AssignmentWithUnaryOperators) {
    auto expr1 = ParseExpression("x = -y");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("x = !flag");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("x = ++count");
    ASSERT_NE(expr3.get(), nullptr);
}

/**
 * @test 测试连续的复合赋值
 */
TEST_F(AssignmentExpressionTest, ConsecutiveCompoundAssignments) {
    auto expr1 = ParseExpression("x += 1; y -= 2");
    // 这应该是表达式语句，不是单个表达式
    ASSERT_NE(expr1.get(), nullptr);
}

/**
 * @test 测试赋值表达式在条件中
 */
TEST_F(AssignmentExpressionTest, AssignmentInCondition) {
    auto expr = ParseExpression("(x = 5) < 10");
    ASSERT_NE(expr.get(), nullptr);
    // 括号内的赋值应该被正确解析
}

/**
 * @test 测试成员访问赋值
 */
TEST_F(AssignmentExpressionTest, MemberAccessAssignment) {
    auto expr1 = ParseExpression("obj.prop = value");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("arr[index] = value");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("obj.prop += 5");
    ASSERT_NE(expr3.get(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

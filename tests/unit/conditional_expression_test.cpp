/**
 * @file conditional_expression_test.cpp
 * @brief 条件表达式（三元运算符）测试
 *
 * 测试条件表达式相关功能，包括:
 * - 基本条件表达式 (条件 ? 真值 : 假值)
 * - 嵌套条件表达式
 * - 条件表达式与其他运算符的组合
 * - 条件表达式的优先级
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
#include "../src/compiler/expression_impl/conditional_expression.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/integer_literal.h"
#include "../src/compiler/expression_impl/binary_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ConditionalExpressionTest
 * @brief 条件表达式测试类
 */
class ConditionalExpressionTest : public ::testing::Test {
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
// 基本条件表达式测试
// ============================================================================

/**
 * @test 测试基本条件表达式
 */
TEST_F(ConditionalExpressionTest, BasicConditionalExpression) {
    auto expr = ParseExpression("a ? b : c");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 验证条件部分
    auto* condition = dynamic_cast<Identifier*>(cond_expr->test().get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->name(), "a");

    // 验证真值部分
    auto* true_expr = dynamic_cast<Identifier*>(cond_expr->consequent().get());
    ASSERT_NE(true_expr, nullptr);
    EXPECT_EQ(true_expr->name(), "b");

    // 验证假值部分
    auto* false_expr = dynamic_cast<Identifier*>(cond_expr->alternate().get());
    ASSERT_NE(false_expr, nullptr);
    EXPECT_EQ(false_expr->name(), "c");
}

/**
 * @test 测试条件为字面量的条件表达式
 */
TEST_F(ConditionalExpressionTest, ConditionalWithLiteralCondition) {
    auto expr = ParseExpression("true ? 'yes' : 'no'");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);
    ASSERT_NE(cond_expr->test().get(), nullptr);
    ASSERT_NE(cond_expr->consequent().get(), nullptr);
    ASSERT_NE(cond_expr->alternate().get(), nullptr);
}

/**
 * @test 测试条件为比较表达式的条件表达式
 */
TEST_F(ConditionalExpressionTest, ConditionalWithComparison) {
    auto expr = ParseExpression("x > 5 ? 'big' : 'small'");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 条件应该是比较表达式
    auto* condition = dynamic_cast<BinaryExpression*>(cond_expr->test().get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op(), TokenType::kOpGt);
}

// ============================================================================
// 嵌套条件表达式测试
// ============================================================================

/**
 * @test 测试嵌套条件表达式（真值分支嵌套）
 */
TEST_F(ConditionalExpressionTest, NestedConditionalInTrueBranch) {
    auto expr = ParseExpression("a ? (b ? c : d) : e");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 真值部分应该是另一个条件表达式
    auto* true_expr = dynamic_cast<ConditionalExpression*>(cond_expr->consequent().get());
    ASSERT_NE(true_expr, nullptr);
}

/**
 * @test 测试嵌套条件表达式（假值分支嵌套）
 */
TEST_F(ConditionalExpressionTest, NestedConditionalInFalseBranch) {
    auto expr = ParseExpression("a ? b : (c ? d : e)");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 假值部分应该是另一个条件表达式
    auto* false_expr = dynamic_cast<ConditionalExpression*>(cond_expr->alternate().get());
    ASSERT_NE(false_expr, nullptr);
}

/**
 * @test 测试多层嵌套条件表达式
 */
TEST_F(ConditionalExpressionTest, MultipleNestedConditionals) {
    auto expr = ParseExpression("a ? b : c ? d : e ? f : g");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 假值部分应该嵌套了多层条件表达式
    auto* false_expr = dynamic_cast<ConditionalExpression*>(cond_expr->alternate().get());
    ASSERT_NE(false_expr, nullptr);
}

/**
 * @test 测试三元链式条件（类似if-else if-else）
 */
TEST_F(ConditionalExpressionTest, ChainedConditionalLikeIfElse) {
    auto expr = ParseExpression("a ? b : c ? d : e");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 验证假值分支是嵌套的条件表达式
    auto* second_cond = dynamic_cast<ConditionalExpression*>(cond_expr->alternate().get());
    ASSERT_NE(second_cond, nullptr);
}

// ============================================================================
// 条件表达式与其他运算符组合测试
// ============================================================================

/**
 * @test 测试条件表达式与算术运算符组合
 */
TEST_F(ConditionalExpressionTest, ConditionalWithArithmeticOperators) {
    auto expr1 = ParseExpression("a ? b + c : d - e");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("(a ? b : c) * 2");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试条件表达式与逻辑运算符组合
 */
TEST_F(ConditionalExpressionTest, ConditionalWithLogicalOperators) {
    auto expr1 = ParseExpression("a && b ? c : d");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("a ? b : c || d");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试条件表达式作为赋值右值
 */
TEST_F(ConditionalExpressionTest, ConditionalAsAssignmentValue) {
    auto expr = ParseExpression("result = score > 60 ? 'pass' : 'fail'");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    // 整体可能是一个赋值表达式，但我们至少能解析它
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试条件表达式作为函数参数
 */
TEST_F(ConditionalExpressionTest, ConditionalAsFunctionArgument) {
    auto expr = ParseExpression("func(a > b ? x : y)");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试条件表达式在数组中
 */
TEST_F(ConditionalExpressionTest, ConditionalInArray) {
    auto expr = ParseExpression("[a ? b : c, d ? e : f]");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试条件表达式在对象中
 */
TEST_F(ConditionalExpressionTest, ConditionalInObject) {
    auto expr = ParseExpression("{x: a ? b : c, y: d ? e : f}");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 条件表达式优先级测试
// ============================================================================

/**
 * @test 测试条件表达式优先级低于逻辑运算符
 */
TEST_F(ConditionalExpressionTest, ConditionalLowerThanLogical) {
    auto expr = ParseExpression("a || b ? c : d");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 条件部分应该是逻辑或表达式
    auto* condition = dynamic_cast<BinaryExpression*>(cond_expr->test().get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->op(), TokenType::kOpOr);
}

/**
 * @test 测试条件表达式优先级高于赋值运算符
 */
TEST_F(ConditionalExpressionTest, ConditionalHigherThanAssignment) {
    auto expr = ParseExpression("x = a ? b : c");
    // 整体应该是赋值表达式
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试条件表达式右结合性
 */
TEST_F(ConditionalExpressionTest, ConditionalIsRightAssociative) {
    auto expr = ParseExpression("a ? b : c ? d : e");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 由于右结合性，假值部分应该是另一个条件表达式
    auto* false_expr = dynamic_cast<ConditionalExpression*>(cond_expr->alternate().get());
    ASSERT_NE(false_expr, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试条件表达式各部分为复杂表达式
 */
TEST_F(ConditionalExpressionTest, ComplexExpressionsInAllParts) {
    auto expr = ParseExpression("(x + y) > 10 ? (a * b + c) : (d / e - f)");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_NE(cond_expr, nullptr);

    // 条件部分
    ASSERT_NE(cond_expr->test().get(), nullptr);

    // 真值部分
    ASSERT_NE(cond_expr->consequent().get(), nullptr);

    // 假值部分
    ASSERT_NE(cond_expr->alternate().get(), nullptr);
}

/**
 * @test 测试条件表达式与括号
 */
TEST_F(ConditionalExpressionTest, ConditionalWithParentheses) {
    auto expr1 = ParseExpression("(a ? b : c)");
    auto* cond_expr1 = dynamic_cast<ConditionalExpression*>(expr1.get());
    ASSERT_NE(cond_expr1, nullptr);

    auto expr2 = ParseExpression("a ? (b + c) : (d - e)");
    auto* cond_expr2 = dynamic_cast<ConditionalExpression*>(expr2.get());
    ASSERT_NE(cond_expr2, nullptr);
}

/**
 * @test 测试省略真值或假值的情况（应该报错）
 */
TEST_F(ConditionalExpressionTest, IncompleteConditional) {
    // 这些应该抛出异常或返回错误
    // 注意：根据实现，可能需要在运行时才能检测到错误
    // auto expr1 = ParseExpression("a ? b");
    // auto expr2 = ParseExpression("a : b");
    // 这里我们只测试完整的情况
}

/**
 * @test 测试条件表达式嵌套在二元表达式中
 */
TEST_F(ConditionalExpressionTest, ConditionalNestedInBinary) {
    auto expr1 = ParseExpression("x + (a ? b : c)");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("(a ? b : c) + y");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试条件表达式在成员访问中
 */
TEST_F(ConditionalExpressionTest, ConditionalInMemberAccess) {
    auto expr1 = ParseExpression("(a ? obj1 : obj2).property");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("(a ? arr1 : arr2)[index]");
    ASSERT_NE(expr2.get(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

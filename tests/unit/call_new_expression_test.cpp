/**
 * @file call_new_expression_test.cpp
 * @brief 函数调用和new表达式测试
 *
 * 测试函数调用和new表达式，包括:
 * - 简单函数调用
 * - 方法调用
 * - 构造函数调用 (new)
 * - 嵌套调用
 * - Call/Apply方法调用
 * - 可选链调用
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
#include "../src/compiler/expression_impl/call_expression.h"
#include "../src/compiler/expression_impl/new_expression.h"
#include "../src/compiler/expression_impl/member_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class CallNewExpressionTest
 * @brief 函数调用和new表达式测试类
 */
class CallNewExpressionTest : public ::testing::Test {
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
// 函数调用测试 - 基础
// ============================================================================

/**
 * @test 测试无参数函数调用
 */
TEST_F(CallNewExpressionTest, SimpleFunctionCallNoArgs) {
    auto expr = ParseExpression("func()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 0);
}

/**
 * @test 测试单参数函数调用
 */
TEST_F(CallNewExpressionTest, FunctionCallWithSingleArgument) {
    auto expr = ParseExpression("func(x)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 1);
}

/**
 * @test 测试多参数函数调用
 */
TEST_F(CallNewExpressionTest, FunctionCallWithMultipleArguments) {
    auto expr = ParseExpression("func(a, b, c)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 3);
}

/**
 * @test 测试函数调用参数为表达式
 */
TEST_F(CallNewExpressionTest, FunctionCallWithExpressionArguments) {
    auto expr = ParseExpression("func(a + b, c * d, e || f)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 3);
}

// ============================================================================
// 方法调用测试
// ============================================================================

/**
 * @test 测试简单方法调用
 */
TEST_F(CallNewExpressionTest, SimpleMethodCall) {
    auto expr = ParseExpression("obj.method()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 0);

    // 验证被调用者是成员表达式
    auto* member_expr = dynamic_cast<MemberExpression*>(call_expr->callee().get());
    ASSERT_NE(member_expr, nullptr);
}

/**
 * @test 测试带参数的方法调用
 */
TEST_F(CallNewExpressionTest, MethodCallWithArguments) {
    auto expr = ParseExpression("obj.method(a, b)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 2);
}

/**
 * @test 测试链式方法调用
 */
TEST_F(CallNewExpressionTest, ChainedMethodCalls) {
    auto expr = ParseExpression("obj.method1().method2()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);

    // 被调用者应该是一个调用表达式的成员访问
    auto* member_expr = dynamic_cast<MemberExpression*>(call_expr->callee().get());
    ASSERT_NE(member_expr, nullptr);
}

/**
 * @test 测试深层链式调用
 */
TEST_F(CallNewExpressionTest, DeepChainedMethodCalls) {
    auto expr = ParseExpression("obj.a().b().c()");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 嵌套调用测试
// ============================================================================

/**
 * @test 测试嵌套函数调用
 */
TEST_F(CallNewExpressionTest, NestedFunctionCalls) {
    auto expr = ParseExpression("func1(func2(x))");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 1);

    // 参数应该也是一个调用表达式
    auto* inner_call = dynamic_cast<CallExpression*>(call_expr->arguments()[0].get());
    ASSERT_NE(inner_call, nullptr);
}

/**
 * @test 测试多层嵌套调用
 */
TEST_F(CallNewExpressionTest, MultiLevelNestedCalls) {
    auto expr = ParseExpression("func1(func2(func3(x)))");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
}

// ============================================================================
// new表达式测试
// ============================================================================

/**
 * @test 测试简单new调用
 */
TEST_F(CallNewExpressionTest, SimpleNewCall) {
    auto expr = ParseExpression("new Constructor()");
    auto* new_expr = dynamic_cast<NewExpression*>(expr.get());
    ASSERT_NE(new_expr, nullptr);
    EXPECT_EQ(new_expr->arguments().size(), 0);
}

/**
 * @test 测试带参数的new调用
 */
TEST_F(CallNewExpressionTest, NewCallWithArguments) {
    auto expr = ParseExpression("new Constructor(a, b, c)");
    auto* new_expr = dynamic_cast<NewExpression*>(expr.get());
    ASSERT_NE(new_expr, nullptr);
    EXPECT_EQ(new_expr->arguments().size(), 3);
}

/**
 * @test 测试new后跟成员访问
 */
TEST_F(CallNewExpressionTest, NewCallWithMemberAccess) {
    auto expr = ParseExpression("new Constructor().prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_NE(member_expr, nullptr);
    EXPECT_FALSE(member_expr->computed());
}

/**
 * @test 测试new后跟方法调用
 */
TEST_F(CallNewExpressionTest, NewCallWithMethodCall) {
    auto expr = ParseExpression("new Constructor().method()");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 特殊调用模式测试
// ============================================================================

/**
 * @test 测试IIFE（立即执行函数表达式）
 */
TEST_F(CallNewExpressionTest, ImmediatelyInvokedFunctionExpression) {
    auto expr = ParseExpression("(function() { return 42; })()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
}

/**
 * @test 测试带参数的IIFE
 */
TEST_F(CallNewExpressionTest, IIFEWithArguments) {
    auto expr = ParseExpression("(function(x, y) { return x + y; })(1, 2)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 2);
}

/**
 * @test 测试箭头函数IIFE
 */
TEST_F(CallNewExpressionTest, ArrowFunctionIIFE) {
    auto expr = ParseExpression("(() => 42)()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
}

/**
 * @test 测试call方法调用
 */
TEST_F(CallNewExpressionTest, CallMethodInvocation) {
    auto expr = ParseExpression("func.call(thisArg, a, b)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 3);

    auto* member_expr = dynamic_cast<MemberExpression*>(call_expr->callee().get());
    ASSERT_NE(member_expr, nullptr);
}

/**
 * @test 测试apply方法调用
 */
TEST_F(CallNewExpressionTest, ApplyMethodInvocation) {
    auto expr = ParseExpression("func.apply(thisArg, argsArray)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 2);
}

// ============================================================================
// 可选链调用测试
// ============================================================================

/**
 * @test 测试可选链方法调用
 */
TEST_F(CallNewExpressionTest, OptionalChainingMethodCall) {
    auto expr = ParseExpression("obj?.method()");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试可选链嵌套调用
 */
TEST_F(CallNewExpressionTest, OptionalChainingNestedCalls) {
    auto expr = ParseExpression("obj?.a?.b?.()");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 复杂场景测试
// ============================================================================

/**
 * @test 测试构造函数中的new
 */
TEST_F(CallNewExpressionTest, NewInConstructor) {
    auto expr = ParseExpression("new new Constructor()");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试函数表达式作为参数
 */
TEST_F(CallNewExpressionTest, FunctionExpressionAsArgument) {
    auto expr = ParseExpression("setTimeout(function() { return 42; }, 1000)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 2);
}

/**
 * @test 测试箭头函数作为参数
 */
TEST_F(CallNewExpressionTest, ArrowFunctionAsArgument) {
    auto expr = ParseExpression("arr.map(x => x * 2)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 1);
}

/**
 * @test 测试对象方法中的this
 */
TEST_F(CallNewExpressionTest, ObjectMethodCall) {
    auto expr = ParseExpression("obj.method()");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);

    auto* member_expr = dynamic_cast<MemberExpression*>(call_expr->callee().get());
    ASSERT_NE(member_expr, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空参数列表
 */
TEST_F(CallNewExpressionTest, EmptyArgumentList) {
    auto expr1 = ParseExpression("func()");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("new Constructor()");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试末尾逗号
 */
TEST_F(CallNewExpressionTest, TrailingCommaInArguments) {
    auto expr = ParseExpression("func(a, b, c,)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 3);
}

/**
 * @test 测试大量参数
 */
TEST_F(CallNewExpressionTest, LargeNumberOfArguments) {
    auto expr = ParseExpression("func(1,2,3,4,5,6,7,8,9,10)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    EXPECT_EQ(call_expr->arguments().size(), 10);
}

/**
 * @test 测试表达式作为被调用者
 */
TEST_F(CallNewExpressionTest, ExpressionAsCallee) {
    auto expr1 = ParseExpression("(cond ? func1 : func2)()");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("obj[methodName]()");
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// 错误情况测试
// ============================================================================

/**
 * @test 测试不匹配的括号
 */
TEST_F(CallNewExpressionTest, UnmatchedParentheses) {
    EXPECT_THROW({
        ParseExpression("func(");
    }, SyntaxError);

    EXPECT_THROW({
        ParseExpression("new Constructor(");
    }, SyntaxError);
}

/**
 * @test 测试空的参数表达式
 */
TEST_F(CallNewExpressionTest, EmptyArgumentExpression) {
    // 连续逗号会产生空洞（在某些实现中）
    auto expr = ParseExpression("func(a, , b)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_NE(call_expr, nullptr);
    // 中间的空参数可能有特殊处理
}

} // namespace test
} // namespace compiler
} // namespace mjs

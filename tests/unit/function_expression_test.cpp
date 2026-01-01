/**
 * @file function_expression_test.cpp
 * @brief 函数表达式测试
 *
 * 测试所有函数表达式类型，包括:
 * - 传统函数表达式 (function expression)
 * - 箭头函数 (arrow function)
 * - 异步函数 (async function)
 * - 生成器函数 (generator function)
 * - 函数参数
 * - 默认参数
 * - 剩余参数
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
#include "../src/compiler/expression_impl/function_expression.h"
#include "../src/compiler/expression_impl/arrow_function_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class FunctionExpressionTest
 * @brief 函数表达式测试类
 */
class FunctionExpressionTest : public ::testing::Test {
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
// 传统函数表达式测试
// ============================================================================

/**
 * @test 测试匿名函数表达式
 */
TEST_F(FunctionExpressionTest, AnonymousFunctionExpression) {
    auto expr = ParseExpression("function() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_TRUE(func_expr->id().empty());
    EXPECT_EQ(func_expr->params().size(), 0);
    EXPECT_FALSE(func_expr->is_generator());
    EXPECT_FALSE(func_expr->is_async());
}

/**
 * @test 测试命名函数表达式
 */
TEST_F(FunctionExpressionTest, NamedFunctionExpression) {
    auto expr = ParseExpression("function foo() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->id(), "foo");
    EXPECT_EQ(func_expr->params().size(), 0);
}

/**
 * @test 测试带参数的函数表达式
 */
TEST_F(FunctionExpressionTest, FunctionExpressionWithParameters) {
    auto expr = ParseExpression("function(x, y, z) {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->params().size(), 3);
    EXPECT_EQ(func_expr->params()[0], "x");
    EXPECT_EQ(func_expr->params()[1], "y");
    EXPECT_EQ(func_expr->params()[2], "z");
}

/**
 * @test 测试函数表达式体
 */
TEST_F(FunctionExpressionTest, FunctionExpressionBody) {
    auto expr = ParseExpression("function() { return 42; }");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    // 验证函数体存在（BlockStatement）
}

// ============================================================================
// 异步函数表达式测试
// ============================================================================

/**
 * @test 测试异步函数表达式
 */
TEST_F(FunctionExpressionTest, AsyncFunctionExpression) {
    auto expr = ParseExpression("async function() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_TRUE(func_expr->is_async());
    EXPECT_FALSE(func_expr->is_generator());
}

/**
 * @test 测试命名异步函数表达式
 */
TEST_F(FunctionExpressionTest, NamedAsyncFunctionExpression) {
    auto expr = ParseExpression("async function foo() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->id(), "foo");
    EXPECT_TRUE(func_expr->is_async());
}

/**
 * @test 测试带参数的异步函数表达式
 */
TEST_F(FunctionExpressionTest, AsyncFunctionExpressionWithParameters) {
    auto expr = ParseExpression("async function(x, y) { await x; }");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_TRUE(func_expr->is_async());
    EXPECT_EQ(func_expr->params().size(), 2);
}

// ============================================================================
// 生成器函数表达式测试
// ============================================================================

/**
 * @test 测试生成器函数表达式
 */
TEST_F(FunctionExpressionTest, GeneratorFunctionExpression) {
    auto expr = ParseExpression("function*() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_TRUE(func_expr->is_generator());
    EXPECT_FALSE(func_expr->is_async());
}

/**
 * @test 测试命名生成器函数表达式
 */
TEST_F(FunctionExpressionTest, NamedGeneratorFunctionExpression) {
    auto expr = ParseExpression("function* foo() {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->id(), "foo");
    EXPECT_TRUE(func_expr->is_generator());
}

/**
 * @test 测试带参数的生成器函数表达式
 */
TEST_F(FunctionExpressionTest, GeneratorFunctionExpressionWithParameters) {
    auto expr = ParseExpression("function*(x, y) { yield x; }");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_TRUE(func_expr->is_generator());
    EXPECT_EQ(func_expr->params().size(), 2);
}

// ============================================================================
// 箭头函数表达式测试
// ============================================================================

/**
 * @test 测试简单箭头函数（单参数，简洁体）
 */
TEST_F(FunctionExpressionTest, SimpleArrowFunction) {
    auto expr = ParseExpression("x => x");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 1);
    EXPECT_EQ(arrow_func->params()[0], "x");
    EXPECT_FALSE(arrow_func->is_async());
}

/**
 * @test 测试多参数箭头函数
 */
TEST_F(FunctionExpressionTest, ArrowFunctionWithMultipleParameters) {
    auto expr = ParseExpression("(x, y) => x + y");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 2);
    EXPECT_EQ(arrow_func->params()[0], "x");
    EXPECT_EQ(arrow_func->params()[1], "y");
}

/**
 * @test 测试无参数箭头函数
 */
TEST_F(FunctionExpressionTest, ArrowFunctionWithNoParameters) {
    auto expr = ParseExpression("() => 42");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 0);
}

/**
 * @test 测试块体箭头函数
 */
TEST_F(FunctionExpressionTest, ArrowFunctionWithBlockBody) {
    auto expr = ParseExpression("(x, y) => { return x + y; }");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 2);
}

/**
 * @test 测试异步箭头函数
 */
TEST_F(FunctionExpressionTest, AsyncArrowFunction) {
    auto expr = ParseExpression("async x => await x");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_TRUE(arrow_func->is_async());
    EXPECT_EQ(arrow_func->params().size(), 1);
}

/**
 * @test 测试异步多参数箭头函数
 */
TEST_F(FunctionExpressionTest, AsyncArrowFunctionWithMultipleParameters) {
    auto expr = ParseExpression("async (x, y) => await x + y");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_TRUE(arrow_func->is_async());
    EXPECT_EQ(arrow_func->params().size(), 2);
}

// ============================================================================
// 函数参数测试
// ============================================================================

/**
 * @test 测试默认参数
 */
TEST_F(FunctionExpressionTest, FunctionWithDefaultParameters) {
    auto expr = ParseExpression("function(x = 1, y = 2) {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->params().size(), 2);
}

/**
 * @test 测试箭头函数默认参数
 */
TEST_F(FunctionExpressionTest, ArrowFunctionWithDefaultParameters) {
    auto expr = ParseExpression("(x = 1, y = 2) => x + y");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 2);
}

/**
 * @test 测试剩余参数
 */
TEST_F(FunctionExpressionTest, FunctionWithRestParameters) {
    auto expr = ParseExpression("function(...args) {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->params().size(), 1);
}

/**
 * @test 测试混合参数（普通参数 + 剩余参数）
 */
TEST_F(FunctionExpressionTest, FunctionWithMixedParameters) {
    auto expr = ParseExpression("function(a, b, ...rest) {}");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_NE(func_expr, nullptr);
    EXPECT_EQ(func_expr->params().size(), 3);
}

// ============================================================================
// 复杂场景测试
// ============================================================================

/**
 * @test 测试函数表达式作为值
 */
TEST_F(FunctionExpressionTest, FunctionExpressionAsValue) {
    auto expr = ParseExpression("var f = function() {}");
    // 这是一个变量声明语句，不是纯表达式
    // 但函数表达式作为初始值
}

/**
 * @test 测试立即调用函数表达式(IIFE)
 */
TEST_F(FunctionExpressionTest, ImmediatelyInvokedFunctionExpression) {
    auto expr = ParseExpression("(function() { return 42; })()");
    // 函数表达式后跟调用操作符
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试箭头函数作为回调
 */
TEST_F(FunctionExpressionTest, ArrowFunctionAsCallback) {
    auto expr = ParseExpression("arr.map(x => x * 2)");
    // 箭头函数作为方法调用的参数
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试嵌套函数表达式
 */
TEST_F(FunctionExpressionTest, NestedFunctionExpressions) {
    auto expr = ParseExpression("function outer() { function inner() {} }");
    // 函数内部的函数定义（声明语句）
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试返回函数的函数
 */
TEST_F(FunctionExpressionTest, FunctionReturningFunction) {
    auto expr = ParseExpression("function() { return function() {}; }");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空函数体的函数
 */
TEST_F(FunctionExpressionTest, FunctionWithEmptyBody) {
    auto expr1 = ParseExpression("function() {}");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("() => {}");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试单个参数的箭头函数（无括号）
 */
TEST_F(FunctionExpressionTest, ArrowFunctionSingleParameterNoParens) {
    auto expr = ParseExpression("x => x * 2");
    auto* arrow_func = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_NE(arrow_func, nullptr);
    EXPECT_EQ(arrow_func->params().size(), 1);
}

/**
 * @test 测试带返回值的箭头函数
 */
TEST_F(FunctionExpressionTest, ArrowFunctionWithReturn) {
    auto expr1 = ParseExpression("x => x + 1");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("x => { return x + 1; }");
    ASSERT_NE(expr2.get(), nullptr);
}

/**
 * @test 测试解构参数
 */
TEST_F(FunctionExpressionTest, FunctionWithDestructuredParameters) {
    auto expr1 = ParseExpression("function({a, b}) {}");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("function([x, y]) {}");
    ASSERT_NE(expr2.get(), nullptr);

    auto expr3 = ParseExpression("({a, b}) => {}");
    ASSERT_NE(expr3.get(), nullptr);
}

// ============================================================================
// 错误情况测试
// ============================================================================

/**
 * @test 测试async生成器函数（应该抛出错误）
 */
TEST_F(FunctionExpressionTest, AsyncGeneratorFunctionShouldFail) {
    // async generator functions 当前不支持
    // 如果实现支持，则移除此测试
    EXPECT_THROW({
        ParseExpression("async function*() {}");
    }, SyntaxError);
}

} // namespace test
} // namespace compiler
} // namespace mjs

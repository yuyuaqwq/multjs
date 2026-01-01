/**
 * @file advanced_expression_test.cpp
 * @brief 高级表达式测试
 *
 * 测试高级表达式，包括:
 * - 模板字符串 (Template literals)
 * - yield表达式
 * - await表达式
 * - import表达式
 * - class表达式
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
#include "../src/compiler/expression_impl/template_literal.h"
#include "../src/compiler/expression_impl/yield_expression.h"
#include "../src/compiler/expression_impl/await_expression.h"
#include "../src/compiler/expression_impl/import_expression.h"
#include "../src/compiler/expression_impl/class_expression.h"
#include "../src/compiler/expression_impl/binary_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class AdvancedExpressionTest
 * @brief 高级表达式测试类
 */
class AdvancedExpressionTest : public ::testing::Test {
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
// 模板字符串测试
// ============================================================================

/**
 * @test 测试简单模板字符串
 */
TEST_F(AdvancedExpressionTest, SimpleTemplateLiteral) {
    auto expr = ParseExpression("`hello`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试带插值的模板字符串
 */
TEST_F(AdvancedExpressionTest, TemplateLiteralWithInterpolation) {
    auto expr = ParseExpression("`hello ${name}`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试多插值模板字符串
 */
TEST_F(AdvancedExpressionTest, TemplateLiteralWithMultipleInterpolations) {
    auto expr = ParseExpression("`hello ${name}, you are ${age} years old`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试多行模板字符串
 */
TEST_F(AdvancedExpressionTest, MultiLineTemplateLiteral) {
    auto expr = ParseExpression("`line1\nline2\nline3`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试嵌套表达式插值
 */
TEST_F(AdvancedExpressionTest, TemplateLiteralWithNestedExpression) {
    auto expr = ParseExpression("`result: ${a + b}`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试模板字符串中的函数调用
 */
TEST_F(AdvancedExpressionTest, TemplateLiteralWithFunctionCall) {
    auto expr = ParseExpression("`result: ${func()}`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试标签模板字符串
 */
TEST_F(AdvancedExpressionTest, TaggedTemplateLiteral) {
    auto expr = ParseExpression("tag`hello ${name}`");
    // 标签模板会被解析为调用表达式
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// yield表达式测试
// ============================================================================

/**
 * @test 测试简单yield表达式
 */
TEST_F(AdvancedExpressionTest, SimpleYieldExpression) {
    auto expr = ParseExpression("yield value");
    auto* yield_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_NE(yield_expr, nullptr);
}

/**
 * @test 测试yield带表达式
 */
TEST_F(AdvancedExpressionTest, YieldWithExpression) {
    auto expr = ParseExpression("yield x + y");
    auto* yield_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_NE(yield_expr, nullptr);
}

/**
 * @test 测试yield*委托表达式
 */
TEST_F(AdvancedExpressionTest, YieldDelegateExpression) {
    auto expr = ParseExpression("yield* iterable");
    auto* yield_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_NE(yield_expr, nullptr);
    EXPECT_TRUE(yield_expr->is_delegate());
}

/**
 * @test 测试yield不带值
 */
TEST_F(AdvancedExpressionTest, YieldWithoutValue) {
    auto expr = ParseExpression("yield");
    auto* yield_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_NE(yield_expr, nullptr);
}

/**
 * @test 测试yield在复杂表达式中
 */
TEST_F(AdvancedExpressionTest, YieldInComplexExpression) {
    auto expr1 = ParseExpression("yield (x + y)");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("yield* generator()");
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// await表达式测试
// ============================================================================

/**
 * @test 测试简单await表达式
 */
TEST_F(AdvancedExpressionTest, SimpleAwaitExpression) {
    auto expr = ParseExpression("await promise");
    auto* await_expr = dynamic_cast<AwaitExpression*>(expr.get());
    ASSERT_NE(await_expr, nullptr);
}

/**
 * @test 测试await带函数调用
 */
TEST_F(AdvancedExpressionTest, AwaitWithFunctionCall) {
    auto expr = ParseExpression("await asyncFunc()");
    auto* await_expr = dynamic_cast<AwaitExpression*>(expr.get());
    ASSERT_NE(await_expr, nullptr);
}

/**
 * @test 测试await带表达式
 * 注意: await优先级低于成员访问但高于加法,所以await x + y是(await x) + y
 */
TEST_F(AdvancedExpressionTest, AwaitWithExpression) {
    // await promise1 + promise2 应该被解析为 (await promise1) + promise2
    // 这是一个BinaryExpression,左边是AwaitExpression
    auto expr = ParseExpression("await promise1 + promise2");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary_expr, nullptr);

    // 检查左操作数是AwaitExpression
    auto* await_expr = dynamic_cast<AwaitExpression*>(binary_expr->left().get());
    ASSERT_NE(await_expr, nullptr);
}

/**
 * @test 测试await在复杂表达式中
 */
TEST_F(AdvancedExpressionTest, AwaitInComplexExpression) {
    auto expr1 = ParseExpression("await (promise)");
    ASSERT_NE(expr1.get(), nullptr);

    auto expr2 = ParseExpression("await fetch(url).json()");
    ASSERT_NE(expr2.get(), nullptr);
}

// ============================================================================
// import表达式测试
// ============================================================================

/**
 * @test 测试动态import表达式
 */
TEST_F(AdvancedExpressionTest, DynamicImportExpression) {
    auto expr = ParseExpression("import('module')");
    auto* import_expr = dynamic_cast<ImportExpression*>(expr.get());
    ASSERT_NE(import_expr, nullptr);
}

/**
 * @test 测试import带变量
 */
TEST_F(AdvancedExpressionTest, ImportWithVariable) {
    auto expr = ParseExpression("import(moduleName)");
    auto* import_expr = dynamic_cast<ImportExpression*>(expr.get());
    ASSERT_NE(import_expr, nullptr);
}

/**
 * @test 测试import带表达式
 */
TEST_F(AdvancedExpressionTest, ImportWithExpression) {
    auto expr = ParseExpression("import('./modules/' + name)");
    auto* import_expr = dynamic_cast<ImportExpression*>(expr.get());
    ASSERT_NE(import_expr, nullptr);
}

// ============================================================================
// class表达式测试
// ============================================================================

/**
 * @test 测试简单class表达式
 */
TEST_F(AdvancedExpressionTest, SimpleClassExpression) {
    auto expr = ParseExpression("class {}");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试命名class表达式
 */
TEST_F(AdvancedExpressionTest, NamedClassExpression) {
    auto expr = ParseExpression("class MyClass {}");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带构造函数
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithConstructor) {
    auto expr = ParseExpression("class { constructor() {} }");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带方法
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithMethods) {
    auto expr = ParseExpression("class { method() {} }");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带继承
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithExtends) {
    auto expr = ParseExpression("class extends Parent {}");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式命名带继承
 */
TEST_F(AdvancedExpressionTest, NamedClassExpressionWithExtends) {
    auto expr = ParseExpression("class Child extends Parent {}");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带getter
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithGetter) {
    auto expr = ParseExpression("class { get prop() { return value; } }");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带setter
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithSetter) {
    auto expr = ParseExpression("class { set prop(value) {} }");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

/**
 * @test 测试class表达式带静态方法
 */
TEST_F(AdvancedExpressionTest, ClassExpressionWithStaticMethod) {
    auto expr = ParseExpression("class { static method() {} }");
    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
    ASSERT_NE(class_expr, nullptr);
}

// ============================================================================
// 复杂场景测试
// ============================================================================

/**
 * @test 测试嵌套模板字符串
 */
TEST_F(AdvancedExpressionTest, NestedTemplateLiterals) {
    auto expr = ParseExpression("`outer ${`inner ${x}` end`");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试yield在箭头函数中
 */
TEST_F(AdvancedExpressionTest, YieldInArrowFunction) {
    // 箭头函数不能是生成器，这应该是错误
    // 但测试边界情况
    auto expr = ParseExpression("() => yield value");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试await在箭头函数中
 */
TEST_F(AdvancedExpressionTest, AwaitInArrowFunction) {
    auto expr = ParseExpression("async () => await promise");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试class表达式立即实例化
 */
TEST_F(AdvancedExpressionTest, ClassExpressionImmediateInstantiation) {
    auto expr = ParseExpression("new (class {})");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试class表达式作为函数参数
 */
TEST_F(AdvancedExpressionTest, ClassExpressionAsArgument) {
    auto expr = ParseExpression("register(class {})");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空模板字符串
 */
TEST_F(AdvancedExpressionTest, EmptyTemplateLiteral) {
    auto expr = ParseExpression("``");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试只有插值的模板字符串
 */
TEST_F(AdvancedExpressionTest, TemplateLiteralWithOnlyInterpolation) {
    auto expr = ParseExpression("`${value}`");
    auto* template_lit = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_NE(template_lit, nullptr);
}

/**
 * @test 测试yield*在复杂表达式中
 */
TEST_F(AdvancedExpressionTest, YieldDelegateInComplexExpression) {
    auto expr = ParseExpression("yield* (a + b ? gen1() : gen2())");
    ASSERT_NE(expr.get(), nullptr);
}

/**
 * @test 测试await链式调用
 */
TEST_F(AdvancedExpressionTest, AwaitChainedCalls) {
    auto expr = ParseExpression("await await promise");
    ASSERT_NE(expr.get(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

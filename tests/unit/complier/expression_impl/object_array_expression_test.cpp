/**
 * @file object_array_expression_test.cpp
 * @brief 对象和数组表达式测试
 *
 * 测试对象和数组表达式，包括:
 * - 空数组和空对象
 * - 数组元素
 * - 对象属性
 * - 嵌套对象和数组
 * - Spread运算符
 * - 简写属性
 * - 计算属性名
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
#include "src/compiler/expression_impl/array_expression.h"
#include "src/compiler/expression_impl/object_expression.h"

using PropertyKind = mjs::compiler::PropertyKind;

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ObjectArrayExpressionTest
 * @brief 对象和数组表达式测试类
 */
class ObjectArrayExpressionTest : public ::testing::Test {
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
// 数组表达式测试 - 基础
// ============================================================================

/**
 * @test 测试空数组
 */
TEST_F(ObjectArrayExpressionTest, EmptyArray) {
    auto expr = ParseExpression("[]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 0);
}

/**
 * @test 测试单元素数组
 */
TEST_F(ObjectArrayExpressionTest, SingleElementArray) {
    auto expr = ParseExpression("[1]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 1);
}

/**
 * @test 测试多元素数组
 */
TEST_F(ObjectArrayExpressionTest, MultipleElementsArray) {
    auto expr = ParseExpression("[1, 2, 3]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 3);
}

/**
 * @test 测试混合类型数组
 */
TEST_F(ObjectArrayExpressionTest, MixedTypeArray) {
    auto expr = ParseExpression("[1, 'hello', true, null]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 4);
}

// ============================================================================
// 数组表达式测试 - 高级特性
// ============================================================================

/**
 * @test 测试稀疏数组（空洞）
 */
TEST_F(ObjectArrayExpressionTest, SparseArray) {
    auto expr = ParseExpression("[1, , , 4]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 4);
    // 中间的空洞元素应该为nullptr
    EXPECT_EQ(array_expr->elements()[1].get(), nullptr);
    EXPECT_EQ(array_expr->elements()[2].get(), nullptr);
}

/**
 * @test 测试数组末尾逗号
 */
TEST_F(ObjectArrayExpressionTest, TrailingCommaInArray) {
    auto expr = ParseExpression("[1, 2, 3,]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 3);
}

/**
 * @test 测试嵌套数组
 */
TEST_F(ObjectArrayExpressionTest, NestedArrays) {
    auto expr = ParseExpression("[[1, 2], [3, 4]]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 2);

    // 验证嵌套的数组
    auto* nested1 = dynamic_cast<ArrayExpression*>(array_expr->elements()[0].get());
    ASSERT_NE(nested1, nullptr);
    EXPECT_EQ(nested1->elements().size(), 2);
}

/**
 * @test 测试数组中的表达式
 */
TEST_F(ObjectArrayExpressionTest, ArrayWithExpressions) {
    auto expr = ParseExpression("[x + y, a * b, c || d]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 3);
}

// ============================================================================
// 对象表达式测试 - 基础
// ============================================================================

/**
 * @test 测试空对象
 */
TEST_F(ObjectArrayExpressionTest, EmptyObject) {
    auto expr = ParseExpression("{}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 0);
}

/**
 * @test 测试单属性对象
 */
TEST_F(ObjectArrayExpressionTest, SinglePropertyObject) {
    auto expr = ParseExpression("{a: 1}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_EQ(object_expr->properties()[0].key, "a");
}

/**
 * @test 测试多属性对象
 */
TEST_F(ObjectArrayExpressionTest, MultiplePropertiesObject) {
    auto expr = ParseExpression("{a: 1, b: 2, c: 3}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 3);
    EXPECT_EQ(object_expr->properties()[0].key, "a");
    EXPECT_EQ(object_expr->properties()[1].key, "b");
    EXPECT_EQ(object_expr->properties()[2].key, "c");
}

// ============================================================================
// 对象表达式测试 - 高级特性
// ============================================================================

/**
 * @test 测试简写属性
 */
TEST_F(ObjectArrayExpressionTest, ShorthandProperties) {
    auto expr = ParseExpression("{a, b, c}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 3);
    EXPECT_TRUE(object_expr->properties()[0].shorthand);
    EXPECT_TRUE(object_expr->properties()[1].shorthand);
    EXPECT_TRUE(object_expr->properties()[2].shorthand);
}

/**
 * @test 测试混合普通属性和简写属性
 */
TEST_F(ObjectArrayExpressionTest, MixedNormalAndShorthandProperties) {
    auto expr = ParseExpression("{a, b: 2, c}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 3);
    EXPECT_TRUE(object_expr->properties()[0].shorthand);  // a
    EXPECT_FALSE(object_expr->properties()[1].shorthand); // b: 2
    EXPECT_TRUE(object_expr->properties()[2].shorthand);  // c
}

/**
 * @test 测试字符串键属性
 */
TEST_F(ObjectArrayExpressionTest, StringKeyProperties) {
    auto expr = ParseExpression("{\"key1\": 1, \"key2\": 2}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 2);
    EXPECT_EQ(object_expr->properties()[0].key, "key1");
    EXPECT_EQ(object_expr->properties()[1].key, "key2");
}

/**
 * @test 测试计算属性名
 */
TEST_F(ObjectArrayExpressionTest, ComputedPropertyNames) {
    auto expr = ParseExpression("{[key]: value}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_TRUE(object_expr->properties()[0].computed);
}

/**
 * @test 测试混合计算属性和普通属性
 */
TEST_F(ObjectArrayExpressionTest, MixedComputedAndNormalProperties) {
    auto expr = ParseExpression("{a: 1, [b]: 2, c: 3}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 3);
    EXPECT_FALSE(object_expr->properties()[0].computed);
    EXPECT_TRUE(object_expr->properties()[1].computed);
    EXPECT_FALSE(object_expr->properties()[2].computed);
}

/**
 * @test 测试嵌套对象
 */
TEST_F(ObjectArrayExpressionTest, NestedObjects) {
    auto expr = ParseExpression("{outer: {inner: 1}}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_EQ(object_expr->properties()[0].key, "outer");

    // 验证嵌套的对象
    auto* nested = dynamic_cast<ObjectExpression*>(object_expr->properties()[0].value.get());
    ASSERT_NE(nested, nullptr);
    EXPECT_EQ(nested->properties().size(), 1);
}

// ============================================================================
// 数组和对象组合测试
// ============================================================================

/**
 * @test 测试对象中的数组
 */
TEST_F(ObjectArrayExpressionTest, ArrayInObject) {
    auto expr = ParseExpression("{arr: [1, 2, 3]}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);

    auto* array_expr = dynamic_cast<ArrayExpression*>(object_expr->properties()[0].value.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 3);
}

/**
 * @test 测试数组中的对象
 */
TEST_F(ObjectArrayExpressionTest, ObjectInArray) {
    auto expr = ParseExpression("[{a: 1}, {b: 2}]");
    auto* array_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_NE(array_expr, nullptr);
    EXPECT_EQ(array_expr->elements().size(), 2);

    auto* obj1 = dynamic_cast<ObjectExpression*>(array_expr->elements()[0].get());
    ASSERT_NE(obj1, nullptr);
    EXPECT_EQ(obj1->properties().size(), 1);

    auto* obj2 = dynamic_cast<ObjectExpression*>(array_expr->elements()[1].get());
    ASSERT_NE(obj2, nullptr);
    EXPECT_EQ(obj2->properties().size(), 1);
}

/**
 * @test 测试复杂的嵌套结构
 */
TEST_F(ObjectArrayExpressionTest, ComplexNestedStructure) {
    auto expr = ParseExpression("{data: {items: [1, 2, 3], count: 3}}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试对象末尾逗号
 */
TEST_F(ObjectArrayExpressionTest, TrailingCommaInObject) {
    auto expr = ParseExpression("{a: 1, b: 2,}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 2);
}

/**
 * @test 测试空的计算属性
 */
TEST_F(ObjectArrayExpressionTest, EmptyComputedProperty) {
    // 空的计算属性名是不合法的，但测试边界情况
    EXPECT_THROW({
        ParseExpression("{[]: value}");
    }, SyntaxError);
}

/**
 * @test 测试对象中的函数表达式
 */
TEST_F(ObjectArrayExpressionTest, FunctionExpressionInObject) {
    auto expr = ParseExpression("{method: function() { return 42; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
}

/**
 * @test 测试对象中的箭头函数
 */
TEST_F(ObjectArrayExpressionTest, ArrowFunctionInObject) {
    auto expr = ParseExpression("{method: () => 42}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
}

/**
 * @test 测试对象中的复杂表达式
 */
TEST_F(ObjectArrayExpressionTest, ComplexExpressionInObject) {
    auto expr = ParseExpression("{sum: a + b, product: a * b}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 2);
}

// ============================================================================
// 对象 getter/setter 测试
// ============================================================================

/**
 * @test 测试对象中的 getter
 */
TEST_F(ObjectArrayExpressionTest, GetterInObject) {
    auto expr = ParseExpression("{get area() { return this.width * this.height; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_EQ(object_expr->properties()[0].key, "area");
    EXPECT_EQ(object_expr->properties()[0].kind, PropertyKind::kGetter);
    EXPECT_FALSE(object_expr->properties()[0].computed);
}

/**
 * @test 测试对象中的 setter
 */
TEST_F(ObjectArrayExpressionTest, SetterInObject) {
    auto expr = ParseExpression("{set width(value) { this._width = value; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_EQ(object_expr->properties()[0].key, "width");
    EXPECT_EQ(object_expr->properties()[0].kind, PropertyKind::kSetter);
    EXPECT_FALSE(object_expr->properties()[0].computed);
}

/**
 * @test 测试对象中同时包含 getter 和 setter
 */
TEST_F(ObjectArrayExpressionTest, GetterAndSetterInObject) {
    auto expr = ParseExpression("{get x() { return _x; }, set x(value) { _x = value; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 2);

    EXPECT_EQ(object_expr->properties()[0].key, "x");
    EXPECT_EQ(object_expr->properties()[0].kind, PropertyKind::kGetter);

    EXPECT_EQ(object_expr->properties()[1].key, "x");
    EXPECT_EQ(object_expr->properties()[1].kind, PropertyKind::kSetter);
}

/**
 * @test 测试混合普通属性和 getter/setter
 */
TEST_F(ObjectArrayExpressionTest, MixedNormalAndGetterSetter) {
    auto expr = ParseExpression("{name: 'test', get value() { return _value; }, set value(v) { _value = v; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 3);

    EXPECT_EQ(object_expr->properties()[0].key, "name");
    EXPECT_EQ(object_expr->properties()[0].kind, PropertyKind::kNormal);

    EXPECT_EQ(object_expr->properties()[1].key, "value");
    EXPECT_EQ(object_expr->properties()[1].kind, PropertyKind::kGetter);

    EXPECT_EQ(object_expr->properties()[2].key, "value");
    EXPECT_EQ(object_expr->properties()[2].kind, PropertyKind::kSetter);
}

/**
 * @test 测试计算属性名的 getter（暂不支持）
 */
TEST_F(ObjectArrayExpressionTest, ComputedGetterNotSupported) {
    // 计算属性名的 getter 暂不支持，应该抛出异常
    EXPECT_THROW({
        ParseExpression("{get [expr]() { return value; }}");
    }, SyntaxError);
}

/**
 * @test 测试计算属性名的 setter（暂不支持）
 */
TEST_F(ObjectArrayExpressionTest, ComputedSetterNotSupported) {
    // 计算属性名的 setter 暂不支持，应该抛出异常
    EXPECT_THROW({
        ParseExpression("{set [expr](value) { _value = value; }}");
    }, SyntaxError);
}

/**
 * @test 测试简单的 getter 返回常量
 */
TEST_F(ObjectArrayExpressionTest, SimpleGetter) {
    auto expr = ParseExpression("{get clrType() { return MessageId; }}");
    auto* object_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_NE(object_expr, nullptr);
    EXPECT_EQ(object_expr->properties().size(), 1);
    EXPECT_EQ(object_expr->properties()[0].key, "clrType");
    EXPECT_EQ(object_expr->properties()[0].kind, PropertyKind::kGetter);
}

} // namespace test
} // namespace compiler
} // namespace mjs

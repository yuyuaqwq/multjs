/**
 * @file class_expression_test.cpp
 * @brief 类表达式测试
 *
 * 测试所有类表达式类型，包括:
 * - 基础类声明和表达式
 * - 构造函数
 * - 实例方法和静态方法
 * - getter和setter
 * - 类字段
 * - 静态字段
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
#include "src/compiler/expression_impl/class_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ClassExpressionTest
 * @brief 类表达式测试类
 */
class ClassExpressionTest : public ::testing::Test {
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

    /**
     * @brief 辅助方法：解析类表达式
     * @param source 源代码字符串
     * @return ClassExpression对象的指针
     */
    std::unique_ptr<ClassExpression> ParseClassExpression(const std::string& source) {
        return std::unique_ptr<ClassExpression>(dynamic_cast<ClassExpression*>(ParseExpression(source).release()));
    }
};

// ============================================================================
// 基础类表达式测试
// ============================================================================

/**
 * @test 测试简单的匿名类表达式
 */
TEST_F(ClassExpressionTest, SimpleAnonymousClass) {
    auto class_expr = ParseClassExpression("class {}");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_FALSE(class_expr->id().has_value());
    EXPECT_FALSE(class_expr->has_super_class());
    EXPECT_EQ(class_expr->elements().size(), 0);
}

/**
 * @test 测试命名类表达式
 */
TEST_F(ClassExpressionTest, NamedClass) {
    auto class_expr = ParseClassExpression("class MyClass {}");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_TRUE(class_expr->id().has_value());
    EXPECT_EQ(class_expr->id().value(), "MyClass");
    EXPECT_FALSE(class_expr->has_super_class());
}

/**
 * @test 测试类表达式作为变量赋值
 */
TEST_F(ClassExpressionTest, ClassAsVariableAssignment) {
    auto expr = ParseExpression("let MyClass = class {}");
    ASSERT_NE(expr.get(), nullptr);
}

// ============================================================================
// 构造函数测试
// ============================================================================

/**
 * @test 测试带构造函数的类
 */
TEST_F(ClassExpressionTest, ClassWithConstructor) {
    auto class_expr = ParseClassExpression(R"(
        class Point {
            constructor(x, y) {
                this.x = x;
                this.y = y;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);

    const auto& elem = class_expr->elements()[0];
    EXPECT_EQ(elem.kind(), MethodKind::kConstructor);
    EXPECT_EQ(elem.key(), "constructor");
}

/**
 * @test 测试无参数构造函数
 */
TEST_F(ClassExpressionTest, ConstructorWithNoParameters) {
    auto class_expr = ParseClassExpression(R"(
        class Simple {
            constructor() {
                this.value = 42;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kConstructor);
}

/**
 * @test 测试带默认参数的构造函数
 */
TEST_F(ClassExpressionTest, ConstructorWithDefaultParameters) {
    auto class_expr = ParseClassExpression(R"(
        class Point {
            constructor(x = 0, y = 0) {
                this.x = x;
                this.y = y;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
}

// ============================================================================
// 实例方法测试
// ============================================================================

/**
 * @test 测试带实例方法的类
 */
TEST_F(ClassExpressionTest, ClassWithInstanceMethods) {
    auto class_expr = ParseClassExpression(R"(
        class Calculator {
            add(a, b) {
                return a + b;
            }

            subtract(a, b) {
                return a - b;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kMethod);
    EXPECT_EQ(class_expr->elements()[0].key(), "add");
    EXPECT_FALSE(class_expr->elements()[0].is_static());

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kMethod);
    EXPECT_EQ(class_expr->elements()[1].key(), "subtract");
    EXPECT_FALSE(class_expr->elements()[1].is_static());
}

/**
 * @test 测试无参数方法
 */
TEST_F(ClassExpressionTest, MethodWithNoParameters) {
    auto class_expr = ParseClassExpression(R"(
        class Greeter {
            greet() {
                return 'Hello';
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].key(), "greet");
}

// ============================================================================
// 静态方法测试
// ============================================================================

/**
 * @test 测试带静态方法的类
 */
TEST_F(ClassExpressionTest, ClassWithStaticMethods) {
    auto class_expr = ParseClassExpression(R"(
        class MathUtil {
            static add(a, b) {
                return a + b;
            }

            static multiply(a, b) {
                return a * b;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kStatic);
    EXPECT_EQ(class_expr->elements()[0].key(), "add");
    EXPECT_TRUE(class_expr->elements()[0].is_static());

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kStatic);
    EXPECT_EQ(class_expr->elements()[1].key(), "multiply");
    EXPECT_TRUE(class_expr->elements()[1].is_static());
}

/**
 * @test 测试混合静态和实例方法
 */
TEST_F(ClassExpressionTest, MixedStaticAndInstanceMethods) {
    auto class_expr = ParseClassExpression(R"(
        class Example {
            instanceMethod() {
                return 'instance';
            }

            static staticMethod() {
                return 'static';
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_FALSE(class_expr->elements()[0].is_static());
    EXPECT_TRUE(class_expr->elements()[1].is_static());
}

// ============================================================================
// Getter和Setter测试
// ============================================================================

/**
 * @test 测试带getter的类
 */
TEST_F(ClassExpressionTest, ClassWithGetter) {
    auto class_expr = ParseClassExpression(R"(
        class Rectangle {
            get area() {
                return this.width * this.height;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kGetter);
    EXPECT_EQ(class_expr->elements()[0].key(), "area");
    EXPECT_FALSE(class_expr->elements()[0].is_static());
}

/**
 * @test 测试带setter的类
 */
TEST_F(ClassExpressionTest, ClassWithSetter) {
    auto class_expr = ParseClassExpression(R"(
        class Rectangle {
            set width(value) {
                this._width = value;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kSetter);
    EXPECT_EQ(class_expr->elements()[0].key(), "width");
    EXPECT_FALSE(class_expr->elements()[0].is_static());
}

/**
 * @test 测试带getter和setter的类
 */
TEST_F(ClassExpressionTest, ClassWithGetterAndSetter) {
    auto class_expr = ParseClassExpression(R"(
        class Rectangle {
            get area() {
                return this._width * this._height;
            }

            set dimensions(value) {
                this._width = value;
            }

            get dimensions() {
                return this._width;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 3);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kGetter);
    EXPECT_EQ(class_expr->elements()[0].key(), "area");

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kSetter);
    EXPECT_EQ(class_expr->elements()[1].key(), "dimensions");

    EXPECT_EQ(class_expr->elements()[2].kind(), MethodKind::kGetter);
    EXPECT_EQ(class_expr->elements()[2].key(), "dimensions");
}

/**
 * @test 测试带静态getter和setter的类
 */
TEST_F(ClassExpressionTest, ClassWithStaticGetterSetter) {
    auto class_expr = ParseClassExpression(R"(
        class Config {
            static get version() {
                return '1.0.0';
            }

            static set version(value) {
                Config._version = value;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kStaticGetter);
    EXPECT_TRUE(class_expr->elements()[0].is_static());

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kStaticSetter);
    EXPECT_TRUE(class_expr->elements()[1].is_static());
}

// ============================================================================
// 类字段测试
// ============================================================================

/**
 * @test 测试带实例字段的类
 */
TEST_F(ClassExpressionTest, ClassWithInstanceFields) {
    auto class_expr = ParseClassExpression(R"(
        class Point {
            x = 0;
            y = 0;
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[0].key(), "x");
    EXPECT_FALSE(class_expr->elements()[0].is_static());

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[1].key(), "y");
    EXPECT_FALSE(class_expr->elements()[1].is_static());
}

/**
 * @test 测试带初始化值的字段
 */
TEST_F(ClassExpressionTest, ClassWithInitializedFields) {
    auto class_expr = ParseClassExpression(R"(
        class Counter {
            count = 0;
            name = 'counter';
            active = true;
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 3);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[0].key(), "count");

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[1].key(), "name");

    EXPECT_EQ(class_expr->elements()[2].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[2].key(), "active");
}

/**
 * @test 测试带字段和方法的类
 */
TEST_F(ClassExpressionTest, ClassWithFieldsAndMethods) {
    auto class_expr = ParseClassExpression(R"(
        class Point {
            x = 0;
            y = 0;

            constructor(x, y) {
                this.x = x;
                this.y = y;
            }

            getDistance() {
                return Math.sqrt(this.x * this.x + this.y * this.y);
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 4);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kField);
    EXPECT_EQ(class_expr->elements()[2].kind(), MethodKind::kConstructor);
    EXPECT_EQ(class_expr->elements()[3].kind(), MethodKind::kMethod);
}

// ============================================================================
// 静态字段测试
// ============================================================================

/**
 * @test 测试带静态字段的类
 */
TEST_F(ClassExpressionTest, ClassWithStaticFields) {
    auto class_expr = ParseClassExpression(R"(
        class Config {
            static version = '1.0.0';
            static debug = false;
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kStaticField);
    EXPECT_EQ(class_expr->elements()[0].key(), "version");
    EXPECT_TRUE(class_expr->elements()[0].is_static());

    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kStaticField);
    EXPECT_EQ(class_expr->elements()[1].key(), "debug");
    EXPECT_TRUE(class_expr->elements()[1].is_static());
}

/**
 * @test 测试带静态字段和静态方法的类
 */
TEST_F(ClassExpressionTest, ClassWithStaticFieldsAndMethods) {
    auto class_expr = ParseClassExpression(R"(
        class MathUtil {
            static PI = 3.14159;

            static getPI() {
                return MathUtil.PI;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);

    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kStaticField);
    EXPECT_EQ(class_expr->elements()[1].kind(), MethodKind::kStatic);
}

// ============================================================================
// 计算属性名测试
// ============================================================================

/**
 * @test 测试带计算属性名的类
 */
TEST_F(ClassExpressionTest, ClassWithComputedPropertyNames) {
    auto class_expr = ParseClassExpression(R"(
        class MyClass {
            ['methodName']() {
                return 'computed';
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].key(), "methodName");
}

/**
 * @test 测试带字符串计算属性名的类
 */
TEST_F(ClassExpressionTest, ClassWithComputedStringPropertyNames) {
    auto class_expr = ParseClassExpression(R"(
        class MyClass {
            ['myMethod']() {
                return 42;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].key(), "myMethod");
}

// ============================================================================
// 复杂场景测试
// ============================================================================

/**
 * @test 测试完整的类定义
 */
TEST_F(ClassExpressionTest, CompleteClassDefinition) {
    auto class_expr = ParseClassExpression(R"(
        class Rectangle {
            width = 0;
            height = 0;
            static count = 0;

            constructor(width, height) {
                this.width = width;
                this.height = height;
                Rectangle.count += 1;
            }

            get area() {
                return this.width * this.height;
            }

            set dimensions(value) {
                this.width = value;
                this.height = value;
            }

            static getCount() {
                return Rectangle.count;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_GT(class_expr->elements().size(), 0);
}

/**
 * @test 测试嵌套方法定义
 */
TEST_F(ClassExpressionTest, NestedMethodDefinitions) {
    auto class_expr = ParseClassExpression(R"(
        class Outer {
            method() {
                class Inner {
                    value() {
                        return 42;
                    }
                }
                return new Inner();
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
}

// ============================================================================
// 可选分号测试
// ============================================================================

/**
 * @test 测试类元素间的分号（可选）
 */
TEST_F(ClassExpressionTest, OptionalSemicolons) {
    auto class_expr = ParseClassExpression(R"(
        class MyClass {
            x = 0;
            y = 0;

            method() {
                return this.x + this.y;
            };

            static staticMethod() {
                return 'static';
            };
        }
    )");
    ASSERT_NE(class_expr, nullptr);
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空类
 */
TEST_F(ClassExpressionTest, EmptyClass) {
    auto class_expr = ParseClassExpression("class EmptyClass {}");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 0);
}

/**
 * @test 测试只有构造函数的类
 */
TEST_F(ClassExpressionTest, ClassWithOnlyConstructor) {
    auto class_expr = ParseClassExpression(R"(
        class Simple {
            constructor() {
                this.value = 42;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 1);
    EXPECT_EQ(class_expr->elements()[0].kind(), MethodKind::kConstructor);
}

/**
 * @test 测试只有字段的类
 */
TEST_F(ClassExpressionTest, ClassWithOnlyFields) {
    auto class_expr = ParseClassExpression(R"(
        class Data {
            id = 0;
            name = '';
            active = true;
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 3);
}

/**
 * @test 测试只有静态成员的类
 */
TEST_F(ClassExpressionTest, ClassWithOnlyStaticMembers) {
    auto class_expr = ParseClassExpression(R"(
        class Util {
            static version = '1.0';

            static getVersion() {
                return Util.version;
            }
        }
    )");
    ASSERT_NE(class_expr, nullptr);
    EXPECT_EQ(class_expr->elements().size(), 2);
}

// ============================================================================
// 错误情况测试
// ============================================================================

/**
 * @test 测试继承（当前不支持）
 */
TEST_F(ClassExpressionTest, InheritanceShouldFail) {
    // 继承功能当前未实现，应该抛出SyntaxError
    EXPECT_THROW({
        ParseClassExpression("class Child extends Parent {}");
    }, SyntaxError);
}

/**
 * @test 测试空的计算属性名
 */
TEST_F(ClassExpressionTest, EmptyComputedPropertyNameShouldFail) {
    EXPECT_THROW({
        ParseClassExpression("class MyClass { []() {} }");
    }, SyntaxError);
}

} // namespace test
} // namespace compiler
} // namespace mjs

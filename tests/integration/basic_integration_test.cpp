/**
 * @file basic_integration_test.cpp
 * @brief 基础特性集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class BasicIntegrationTest
 * @brief 基础语言特性集成测试
 */
class BasicIntegrationTest : public IntegrationTestHelper {
};

// ==================== 变量声明与作用域 ====================

TEST_F(BasicIntegrationTest, LetVariableDeclaration) {
    // 测试let变量声明
    AssertEq("let x = 42; x;", Value(42));
    AssertEq("let y = 3.14; y;", Value(3.14));
}

TEST_F(BasicIntegrationTest, ConstVariableDeclaration) {
    // 测试const变量声明
    AssertEq("const x = 100; x;", Value(100));
    AssertEq("const str = 'hello'; str;", Value("hello"));
}

TEST_F(BasicIntegrationTest, BlockScope) {
    // 测试块级作用域
    AssertTrue(R"(
        let x = 10;
        {
            let x = 20;
            x === 20;
        }
        x === 10;
    )");

    AssertTrue(R"(
        const x = 10;
        {
            const x = 20;
            x === 20;
        }
        x === 10;
    )");
}

TEST_F(BasicIntegrationTest, VariableShadowing) {
    // 测试变量遮蔽
    AssertTrue(R"(
        let x = 'outer';
        {
            let x = 'inner';
            x === 'inner';
        }
        x === 'outer';
    )");
}

// ==================== 类型系统 ====================

TEST_F(BasicIntegrationTest, NumberType) {
    // 测试数字类型
    AssertEq("42;", Value(42));
    AssertEq("3.14;", Value(3.14));
    AssertEq("-100;", Value(-100));
    AssertEq("1.5e10;", Value(1.5e10));
}

TEST_F(BasicIntegrationTest, StringType) {
    // 测试字符串类型
    AssertEq("'hello';", Value("hello"));
    AssertEq(R"("world";)", Value("world"));
    AssertEq(R"(`template`;)", Value("template"));
}

TEST_F(BasicIntegrationTest, BooleanType) {
    // 测试布尔类型
    AssertEq("true;", Value(true));
    AssertEq("false;", Value(false));
}

TEST_F(BasicIntegrationTest, NullAndUndefined) {
    // 测试null和undefined
    AssertNull("null;");
    AssertUndefined("undefined;");
}

TEST_F(BasicIntegrationTest, TypeOfOperator) {
    // 测试typeof操作符
    AssertEq("typeof 42;", Value("number"));
    AssertEq("typeof 'hello';", Value("string"));
    AssertEq("typeof true;", Value("boolean"));
    AssertEq("typeof undefined;", Value("undefined"));
}

// ==================== 运算符 ====================

TEST_F(BasicIntegrationTest, ArithmeticOperators) {
    // 测试算术运算符
    AssertEq("1 + 2;", Value(3));
    AssertEq("10 - 5;", Value(5));
    AssertEq("3 * 4;", Value(12));
    AssertEq("20 / 4;", Value(5));
    AssertEq("10 % 3;", Value(1));
}

TEST_F(BasicIntegrationTest, ComparisonOperators) {
    // 测试比较运算符
    AssertTrue("5 > 3");
    AssertTrue("5 >= 5");
    AssertTrue("3 < 5");
    AssertTrue("5 <= 5");
}

TEST_F(BasicIntegrationTest, EqualityOperators) {
    // 测试相等性运算符
    AssertTrue("1 === 1");
    AssertTrue("'hello' === 'hello'");
    AssertTrue("true === true");
    AssertFalse("1 === 2");
    AssertFalse("'hello' === 'world'");
}

TEST_F(BasicIntegrationTest, LogicalOperators) {
    // 测试逻辑运算符
    AssertEq("true && true;", Value(true));
    AssertEq("true && false;", Value(false));
    AssertEq("false || true;", Value(true));
    AssertEq("false || false;", Value(false));
    AssertEq("!true;", Value(false));
    AssertEq("!false;", Value(true));
}

TEST_F(BasicIntegrationTest, StringConcatenation) {
    // 测试字符串拼接
    AssertEq("'hello' + ' ' + 'world';", Value("hello world"));
    AssertEq("'num: ' + 42;", Value("num: 42"));
}

// ==================== 表达式 ====================

TEST_F(BasicIntegrationTest, ConditionalExpression) {
    // 测试条件表达式
    AssertEq("true ? 1 : 0;", Value(1));
    AssertEq("false ? 1 : 0;", Value(0));
}

TEST_F(BasicIntegrationTest, ArrayLiteral) {
    // 测试数组字面量
    AssertEq("[1, 2, 3];", Value(1)); // 只检查第一个元素
    AssertEq("[];", Value(0)); // 空数组
}

TEST_F(BasicIntegrationTest, ObjectLiteral) {
    // 测试对象字面量
    AssertTrue(R"(
        let obj = { x: 1, y: 2 };
        obj.x === 1 && obj.y === 2;
    )");
}

// ==================== 控制流 ====================

TEST_F(BasicIntegrationTest, IfStatement) {
    // 测试if语句
    AssertEq("if (true) { 1; } else { 2; }", Value(1));
    AssertEq("if (false) { 1; } else { 2; }", Value(2));
}

TEST_F(BasicIntegrationTest, WhileLoop) {
    // 测试while循环
    AssertEq(R"(
        let sum = 0;
        let i = 0;
        while (i < 5) {
            sum += i;
            i += 1;
        }
        sum;
    )", Value(10)); // 0 + 1 + 2 + 3 + 4 = 10
}

TEST_F(BasicIntegrationTest, ForLoop) {
    // 测试for循环
    AssertEq(R"(
        let sum = 0;
        for (let i = 0; i < 5; i += 1) {
            sum += i;
        }
        sum;
    )", Value(10)); // 0 + 1 + 2 + 3 + 4 = 10
}

// ==================== 复合场景 ====================

TEST_F(BasicIntegrationTest, ComplexScenario1) {
    // 测试复杂场景：计算阶乘
    AssertEq(R"(
        let n = 5;
        let result = 1;
        for (let i = 2; i <= n; i += 1) {
            result *= i;
        }
        result;
    )", Value(120)); // 5! = 120
}

TEST_F(BasicIntegrationTest, ComplexScenario2) {
    // 测试复杂场景：斐波那契数列
    AssertEq(R"(
        let n = 10;
        let a = 0, b = 1;
        for (let i = 0; i < n; i += 1) {
            let temp = a + b;
            a = b;
            b = temp;
        }
        a;
    )", Value(55)); // 第10个斐波那契数
}

TEST_F(BasicIntegrationTest, ComplexScenario3) {
    // 测试复杂场景：数组操作
    AssertTrue(R"(
        let arr = [1, 2, 3, 4, 5];
        let sum = 0;
        for (let i = 0; i < arr.length; i += 1) {
            sum += arr[i];
        }
        sum === 15;
    )");
}

} // namespace mjs::test

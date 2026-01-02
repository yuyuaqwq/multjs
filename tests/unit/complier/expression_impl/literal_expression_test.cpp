/**
 * @file literal_expression_test.cpp
 * @brief 字面量表达式测试
 *
 * 测试所有字面量类型的表达式，包括:
 * - 整数字面量 (IntegerLiteral)
 * - 浮点数字面量 (FloatLiteral)
 * - 字符串字面量 (StringLiteral)
 * - 布尔字面量 (BooleanLiteral)
 * - null字面量 (NullLiteral)
 * - undefined字面量 (UndefinedLiteral)
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/expression.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/float_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/boolean_literal.h"
#include "src/compiler/expression_impl/null_literal.h"
#include "src/compiler/expression_impl/undefined_literal.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class LiteralExpressionTest
 * @brief 字面量表达式测试类
 */
class LiteralExpressionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：创建Parser对象
     * @param source 源代码字符串
     * @return Parser对象的唯一指针
     */
    std::unique_ptr<Parser> CreateParser(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return std::make_unique<Parser>(lexer.release());
    }

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
// 整数字面量测试
// ============================================================================

/**
 * @test 测试十进制整数字面量
 */
TEST_F(LiteralExpressionTest, DecimalIntegerLiterals) {
    // 测试正整数
    auto expr1 = ParseExpression("42");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 42);

    // 测试零
    auto expr2 = ParseExpression("0");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 0);

    // 测试大整数
    auto expr3 = ParseExpression("9223372036854775807"); // INT64_MAX
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 9223372036854775807);

    // 测试负数（注意：词法分析器会把负号识别为运算符）
    auto expr4 = ParseExpression("-42");
    // 这会被解析为一元表达式，这里先测试正数
}

/**
 * @test 测试十六进制整数字面量
 */
TEST_F(LiteralExpressionTest, HexadecimalIntegerLiterals) {
    // 测试基本十六进制
    auto expr1 = ParseExpression("0xFF");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 255);

    // 测试小写十六进制
    auto expr2 = ParseExpression("0xff");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 255);

    // 测试混合大小写
    auto expr3 = ParseExpression("0XaBcD");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 43981);

    // 测试0
    auto expr4 = ParseExpression("0x0");
    auto* int_lit4 = dynamic_cast<IntegerLiteral*>(expr4.get());
    ASSERT_NE(int_lit4, nullptr);
    EXPECT_EQ(int_lit4->value(), 0);
}

/**
 * @test 测试二进制整数字面量
 */
TEST_F(LiteralExpressionTest, BinaryIntegerLiterals) {
    // 测试基本二进制
    auto expr1 = ParseExpression("0b1010");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 10);

    // 测试大写二进制
    auto expr2 = ParseExpression("0B1010");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 10);

    // 测试全1
    auto expr3 = ParseExpression("0b11111111");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 255);

    // 测试0
    auto expr4 = ParseExpression("0b0");
    auto* int_lit4 = dynamic_cast<IntegerLiteral*>(expr4.get());
    ASSERT_NE(int_lit4, nullptr);
    EXPECT_EQ(int_lit4->value(), 0);
}

/**
 * @test 测试八进制整数字面量
 */
TEST_F(LiteralExpressionTest, OctalIntegerLiterals) {
    // 测试基本八进制
    auto expr1 = ParseExpression("0o77");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 63);

    // 测试大写八进制
    auto expr2 = ParseExpression("0O77");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 63);

    // 测试最大数字
    auto expr3 = ParseExpression("0o77777777");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 16777215);

    // 测试0
    auto expr4 = ParseExpression("0o0");
    auto* int_lit4 = dynamic_cast<IntegerLiteral*>(expr4.get());
    ASSERT_NE(int_lit4, nullptr);
    EXPECT_EQ(int_lit4->value(), 0);
}

/**
 * @test 测试带分隔符的整数字面量
 */
TEST_F(LiteralExpressionTest, IntegerLiteralsWithSeparators) {
    // 测试十进制分隔符
    auto expr1 = ParseExpression("1_000_000");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 1000000);

    // 测试十六进制分隔符
    auto expr2 = ParseExpression("0xFF_FF");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 0xFFFF);

    // 测试二进制分隔符
    auto expr3 = ParseExpression("0b1010_1010");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 0b10101010);

    // 测试八进制分隔符
    auto expr4 = ParseExpression("0o77_77");
    auto* int_lit4 = dynamic_cast<IntegerLiteral*>(expr4.get());
    ASSERT_NE(int_lit4, nullptr);
    EXPECT_EQ(int_lit4->value(), 07777); // C++八进制: 07777
}

// ============================================================================
// 浮点数字面量测试
// ============================================================================

/**
 * @test 测试常规浮点数字面量
 */
TEST_F(LiteralExpressionTest, BasicFloatLiterals) {
    // 测试带小数点的浮点数
    auto expr1 = ParseExpression("3.14");
    auto* float_lit1 = dynamic_cast<FloatLiteral*>(expr1.get());
    ASSERT_NE(float_lit1, nullptr);
    EXPECT_DOUBLE_EQ(float_lit1->value(), 3.14);

    // 测试只有小数部分的浮点数
    auto expr2 = ParseExpression("0.5");
    auto* float_lit2 = dynamic_cast<FloatLiteral*>(expr2.get());
    ASSERT_NE(float_lit2, nullptr);
    EXPECT_DOUBLE_EQ(float_lit2->value(), 0.5);

    // 测试只有整数部分的浮点数（需要小数点）
    auto expr3 = ParseExpression("5.");
    auto* float_lit3 = dynamic_cast<FloatLiteral*>(expr3.get());
    ASSERT_NE(float_lit3, nullptr);
    EXPECT_DOUBLE_EQ(float_lit3->value(), 5.0);
}

/**
 * @test 测试科学计数法浮点数字面量
 */
TEST_F(LiteralExpressionTest, ScientificNotationFloatLiterals) {
    // 测试正指数
    auto expr1 = ParseExpression("1e10");
    auto* float_lit1 = dynamic_cast<FloatLiteral*>(expr1.get());
    ASSERT_NE(float_lit1, nullptr);
    EXPECT_DOUBLE_EQ(float_lit1->value(), 1e10);

    // 测试负指数
    auto expr2 = ParseExpression("1.5e-5");
    auto* float_lit2 = dynamic_cast<FloatLiteral*>(expr2.get());
    ASSERT_NE(float_lit2, nullptr);
    EXPECT_DOUBLE_EQ(float_lit2->value(), 1.5e-5);

    // 测试大写E
    auto expr3 = ParseExpression("1E10");
    auto* float_lit3 = dynamic_cast<FloatLiteral*>(expr3.get());
    ASSERT_NE(float_lit3, nullptr);
    EXPECT_DOUBLE_EQ(float_lit3->value(), 1e10);

    // 测试小数+指数
    auto expr4 = ParseExpression("3.14e2");
    auto* float_lit4 = dynamic_cast<FloatLiteral*>(expr4.get());
    ASSERT_NE(float_lit4, nullptr);
    EXPECT_DOUBLE_EQ(float_lit4->value(), 314.0);
}

/**
 * @test 测试特殊浮点数值
 */
TEST_F(LiteralExpressionTest, SpecialFloatValues) {
    // 测试Infinity（作为标识符）
    auto expr1 = ParseExpression("Infinity");
    // Infinity是全局变量，不是字面量

    // 测试NaN（作为标识符）
    auto expr2 = ParseExpression("NaN");
    // NaN是全局变量，不是字面量
}

/**
 * @test 测试带分隔符的浮点数字面量
 */
TEST_F(LiteralExpressionTest, FloatLiteralsWithSeparators) {
    // 测试带分隔符的浮点数
    auto expr1 = ParseExpression("3.14_15");
    auto* float_lit1 = dynamic_cast<FloatLiteral*>(expr1.get());
    ASSERT_NE(float_lit1, nullptr);
    EXPECT_DOUBLE_EQ(float_lit1->value(), 3.1415);

    // 测试指数部分带分隔符
    auto expr2 = ParseExpression("1e1_0");
    auto* float_lit2 = dynamic_cast<FloatLiteral*>(expr2.get());
    ASSERT_NE(float_lit2, nullptr);
    EXPECT_DOUBLE_EQ(float_lit2->value(), 1e10);
}

// ============================================================================
// 字符串字面量测试
// ============================================================================

/**
 * @test 测试基本字符串字面量
 */
TEST_F(LiteralExpressionTest, BasicStringLiterals) {
    // 测试双引号字符串
    auto expr1 = ParseExpression("\"hello\"");
    auto* str_lit1 = dynamic_cast<StringLiteral*>(expr1.get());
    ASSERT_NE(str_lit1, nullptr);
    EXPECT_EQ(str_lit1->value(), "hello");

    // 测试单引号字符串
    auto expr2 = ParseExpression("'world'");
    auto* str_lit2 = dynamic_cast<StringLiteral*>(expr2.get());
    ASSERT_NE(str_lit2, nullptr);
    EXPECT_EQ(str_lit2->value(), "world");

    // 测试空字符串
    auto expr3 = ParseExpression("\"\"");
    auto* str_lit3 = dynamic_cast<StringLiteral*>(expr3.get());
    ASSERT_NE(str_lit3, nullptr);
    EXPECT_EQ(str_lit3->value(), "");

    auto expr4 = ParseExpression("''");
    auto* str_lit4 = dynamic_cast<StringLiteral*>(expr4.get());
    ASSERT_NE(str_lit4, nullptr);
    EXPECT_EQ(str_lit4->value(), "");
}

/**
 * @test 测试字符串转义序列
 */
TEST_F(LiteralExpressionTest, StringEscapeSequences) {
    // 测试换行符 \n
    auto expr1 = ParseExpression("\"hello\\nworld\"");
    auto* str_lit1 = dynamic_cast<StringLiteral*>(expr1.get());
    ASSERT_NE(str_lit1, nullptr);
    EXPECT_EQ(str_lit1->value(), "hello\nworld");

    // 测试制表符 \t
    auto expr2 = ParseExpression("\"hello\\tworld\"");
    auto* str_lit2 = dynamic_cast<StringLiteral*>(expr2.get());
    ASSERT_NE(str_lit2, nullptr);
    EXPECT_EQ(str_lit2->value(), "hello\tworld");

    // 测试回车符 \r
    auto expr3 = ParseExpression("\"hello\\rworld\"");
    auto* str_lit3 = dynamic_cast<StringLiteral*>(expr3.get());
    ASSERT_NE(str_lit3, nullptr);
    EXPECT_EQ(str_lit3->value(), "hello\rworld");

    // 测试反斜杠转义
    auto expr4 = ParseExpression("\"\\\\\"");
    auto* str_lit4 = dynamic_cast<StringLiteral*>(expr4.get());
    ASSERT_NE(str_lit4, nullptr);
    EXPECT_EQ(str_lit4->value(), "\\");

    // 测试引号转义
    auto expr5 = ParseExpression("\"quote: \\\"\"");
    auto* str_lit5 = dynamic_cast<StringLiteral*>(expr5.get());
    ASSERT_NE(str_lit5, nullptr);
    EXPECT_EQ(str_lit5->value(), "quote: \"");

    auto expr6 = ParseExpression("'it\\'s'");
    auto* str_lit6 = dynamic_cast<StringLiteral*>(expr6.get());
    ASSERT_NE(str_lit6, nullptr);
    EXPECT_EQ(str_lit6->value(), "it's");
}

/**
 * @test 测试Unicode转义序列
 */
TEST_F(LiteralExpressionTest, UnicodeEscapeSequences) {
    // 测试基本Unicode转义 \uXXXX
    auto expr1 = ParseExpression("\"\\u2764\"");
    auto* str_lit1 = dynamic_cast<StringLiteral*>(expr1.get());
    ASSERT_NE(str_lit1, nullptr);
    // EXPECT_EQ(str_lit1->value(), u8"\u2764");

    // 测试Unicode大括号转义 \u{XXXXX}
    auto expr2 = ParseExpression("\"\\u{1F600}\"");
    auto* str_lit2 = dynamic_cast<StringLiteral*>(expr2.get());
    ASSERT_NE(str_lit2, nullptr);
    // EXPECT_EQ(str_lit2->value(), u8"\U0001F600");
}

/**
 * @test 测试多行字符串
 */
TEST_F(LiteralExpressionTest, MultiLineStringLiterals) {
    // JavaScript字符串不支持真正的多行（除非使用模板字符串）
    // 但可以包含换行符转义序列
    auto expr1 = ParseExpression("\"line1\\nline2\\nline3\"");
    auto* str_lit1 = dynamic_cast<StringLiteral*>(expr1.get());
    ASSERT_NE(str_lit1, nullptr);
    EXPECT_EQ(str_lit1->value(), "line1\nline2\nline3");
}

// ============================================================================
// 布尔字面量测试
// ============================================================================

/**
 * @test 测试布尔字面量
 */
TEST_F(LiteralExpressionTest, BooleanLiterals) {
    // 测试true
    auto expr1 = ParseExpression("true");
    auto* bool_lit1 = dynamic_cast<BooleanLiteral*>(expr1.get());
    ASSERT_NE(bool_lit1, nullptr);
    EXPECT_TRUE(bool_lit1->value());

    // 测试false
    auto expr2 = ParseExpression("false");
    auto* bool_lit2 = dynamic_cast<BooleanLiteral*>(expr2.get());
    ASSERT_NE(bool_lit2, nullptr);
    EXPECT_FALSE(bool_lit2->value());
}

// ============================================================================
// null和undefined字面量测试
// ============================================================================

/**
 * @test 测试null字面量
 */
TEST_F(LiteralExpressionTest, NullLiteral) {
    auto expr = ParseExpression("null");
    auto* null_lit = dynamic_cast<NullLiteral*>(expr.get());
    ASSERT_NE(null_lit, nullptr);
    // null没有value()方法，只需要验证类型正确
}

/**
 * @test 测试undefined字面量
 */
TEST_F(LiteralExpressionTest, UndefinedLiteral) {
    auto expr = ParseExpression("undefined");
    auto* undefined_lit = dynamic_cast<UndefinedLiteral*>(expr.get());
    ASSERT_NE(undefined_lit, nullptr);
    // undefined没有value()方法，只需要验证类型正确
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试整数字面量边界值
 */
TEST_F(LiteralExpressionTest, IntegerLiteralBoundaryValues) {
    // 测试较大的32位整数(避免超出 stoll 范围的边缘情况)
    auto expr1 = ParseExpression("2147483647");  // INT32_MAX
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 2147483647);

    // 测试较大的负整数
    auto expr2 = ParseExpression("-2147483648");  // -INT32_MAX
    // 注意：这会被解析为一元表达式+整数字面量

    // 测试 0
    auto expr3 = ParseExpression("0");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 0);

    // 测试 1
    auto expr4 = ParseExpression("1");
    auto* int_lit4 = dynamic_cast<IntegerLiteral*>(expr4.get());
    ASSERT_NE(int_lit4, nullptr);
    EXPECT_EQ(int_lit4->value(), 1);
}

/**
 * @test 测试浮点数精度
 */
TEST_F(LiteralExpressionTest, FloatPrecision) {
    // 测试小精度浮点数
    auto expr1 = ParseExpression("0.000000001");
    auto* float_lit1 = dynamic_cast<FloatLiteral*>(expr1.get());
    ASSERT_NE(float_lit1, nullptr);
    EXPECT_NEAR(float_lit1->value(), 1e-9, 1e-15);

    // 测试大数值浮点数
    auto expr2 = ParseExpression("1.7976931348623157e+308");
    auto* float_lit2 = dynamic_cast<FloatLiteral*>(expr2.get());
    ASSERT_NE(float_lit2, nullptr);
    EXPECT_NEAR(float_lit2->value(), 1.7976931348623157e+308, 1e+293);
}

/**
 * @test 测试各种进制组合
 */
TEST_F(LiteralExpressionTest, MixedBaseLiterals) {
    // 十六进制
    auto expr1 = ParseExpression("0xdeadbeef");
    auto* int_lit1 = dynamic_cast<IntegerLiteral*>(expr1.get());
    ASSERT_NE(int_lit1, nullptr);
    EXPECT_EQ(int_lit1->value(), 0xdeadbeef);

    // 二进制
    auto expr2 = ParseExpression("0b101010");
    auto* int_lit2 = dynamic_cast<IntegerLiteral*>(expr2.get());
    ASSERT_NE(int_lit2, nullptr);
    EXPECT_EQ(int_lit2->value(), 42);

    // 八进制
    auto expr3 = ParseExpression("0o755");
    auto* int_lit3 = dynamic_cast<IntegerLiteral*>(expr3.get());
    ASSERT_NE(int_lit3, nullptr);
    EXPECT_EQ(int_lit3->value(), 493);
}

} // namespace test
} // namespace compiler
} // namespace mjs

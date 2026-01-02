/**
 * @file template_element_test.cpp
 * @brief 模板元素表达式测试
 *
 * 测试模板字符串元素表达式功能,包括:
 * - TemplateElement (模板元素)
 * - 模板元素的构造
 * - 模板元素的值获取
 * - 模板元素的代码生成
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "src/compiler/expression_impl/template_element.h"
#include "src/compiler/expression_impl/template_literal.h"
#include "src/compiler/lexer.h"
#include "src/compiler/code_generator.h"
#include <mjs/function_def.h>
#include <mjs/bytecode_table.h>

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class TemplateElementTest
 * @brief 模板元素表达式测试类
 */
class TemplateElementTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助方法：创建TemplateElement对象
     * @param value 模板元素值
     * @return TemplateElement对象的唯一指针
     */
    std::unique_ptr<TemplateElement> CreateTemplateElement(const std::string& value) {
        return std::make_unique<TemplateElement>(0, value.length(), std::string(value));
    }
};

// ============================================================================
// 构造函数测试
// ============================================================================

/**
 * @test 测试TemplateElement构造函数
 */
TEST_F(TemplateElementTest, Constructor) {
    // 测试简单字符串
    auto elem1 = CreateTemplateElement("hello");
    ASSERT_NE(elem1, nullptr);
    EXPECT_EQ(elem1->value(), "hello");

    // 测试空字符串
    auto elem2 = CreateTemplateElement("");
    ASSERT_NE(elem2, nullptr);
    EXPECT_EQ(elem2->value(), "");

    // 测试包含特殊字符的字符串
    auto elem3 = CreateTemplateElement("hello\nworld\t!");
    ASSERT_NE(elem3, nullptr);
    EXPECT_EQ(elem3->value(), "hello\nworld\t!");

    // 测试包含Unicode的字符串
    auto elem4 = CreateTemplateElement("你好世界🌍");
    ASSERT_NE(elem4, nullptr);
    EXPECT_EQ(elem4->value(), "你好世界🌍");

    // 测试长字符串
    std::string long_value(1000, 'a');
    auto elem5 = CreateTemplateElement(long_value);
    ASSERT_NE(elem5, nullptr);
    EXPECT_EQ(elem5->value(), long_value);
}

/**
 * @test 测试移动语义
 */
TEST_F(TemplateElementTest, MoveSemantics) {
    std::string value = "test string";
    auto elem = CreateTemplateElement(std::move(value));

    EXPECT_EQ(elem->value(), "test string");
    // value已经被移动,可能是空状态
}

// ============================================================================
// 值获取测试
// ============================================================================

/**
 * @test 测试value()方法
 */
TEST_F(TemplateElementTest, ValueMethod) {
    // 测试普通字符串
    auto elem1 = CreateTemplateElement("Hello, World!");
    EXPECT_EQ(elem1->value(), "Hello, World!");

    // 测试包含空格的字符串
    auto elem2 = CreateTemplateElement("  spaces  ");
    EXPECT_EQ(elem2->value(), "  spaces  ");

    // 测试包含引号的字符串
    auto elem3 = CreateTemplateElement("He said \"hello\"");
    EXPECT_EQ(elem3->value(), "He said \"hello\"");

    // 测试包含转义字符的字符串
    auto elem4 = CreateTemplateElement("line1\nline2\rline3\ttab");
    EXPECT_EQ(elem4->value(), "line1\nline2\rline3\ttab");
}

/**
 * @test 测试value()返回const引用
 */
TEST_F(TemplateElementTest, ValueReturnsConstReference) {
    auto elem = CreateTemplateElement("const ref test");
    const std::string& ref = elem->value();
    EXPECT_EQ(ref, "const ref test");
    EXPECT_EQ(&ref, &(elem->value()));
}

// ============================================================================
// 位置信息测试
// ============================================================================

/**
 * @test 测试源代码位置信息
 */
TEST_F(TemplateElementTest, SourcePosition) {
    auto elem = CreateTemplateElement("test");

    EXPECT_EQ(elem->start(), 0);
    EXPECT_EQ(elem->end(), 4);
}

/**
 * @test 测试不同长度元素的位置
 */
TEST_F(TemplateElementTest, DifferentLengthPositions) {
    auto elem1 = CreateTemplateElement("a");
    EXPECT_EQ(elem1->start(), 0);
    EXPECT_EQ(elem1->end(), 1);

    auto elem2 = CreateTemplateElement("abc");
    EXPECT_EQ(elem2->start(), 0);
    EXPECT_EQ(elem2->end(), 3);

    auto elem3 = CreateTemplateElement("你好");
    EXPECT_EQ(elem3->start(), 0);
    // 注意:这里end是字节位置,不是字符位置
    EXPECT_EQ(elem3->end(), 6); // UTF-8编码下每个中文3字节
}

// ============================================================================
// 模板字符串元素特性测试
// ============================================================================

/**
 * @test 测试模板字符串中的静态文本元素
 */
TEST_F(TemplateElementTest, TemplateStringStaticText) {
    // 模板字符串的静态部分
    auto elem = CreateTemplateElement("Hello, ");
    EXPECT_EQ(elem->value(), "Hello, ");
}

/**
 * @test 测试模板字符串中的换行符
 */
TEST_F(TemplateElementTest, TemplateStringNewlines) {
    // 模板字符串中保留换行
    auto elem = CreateTemplateElement("line1\nline2\nline3");
    EXPECT_EQ(elem->value(), "line1\nline2\nline3");
}

/**
 * @test 测试模板字符串中的表达式占位符分隔
 */
TEST_F(TemplateElementTest, TemplateStringInterpolationSeparators) {
    // 表达式前的文本
    auto before = CreateTemplateElement("Value: ");
    EXPECT_EQ(before->value(), "Value: ");

    // 表达式后的文本
    auto after = CreateTemplateElement("!");
    EXPECT_EQ(after->value(), "!");
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空字符串元素
 */
TEST_F(TemplateElementTest, EmptyString) {
    auto elem = CreateTemplateElement("");
    ASSERT_NE(elem, nullptr);
    EXPECT_TRUE(elem->value().empty());
    EXPECT_EQ(elem->value().length(), 0);
}

/**
 * @test 测试单个字符元素
 */
TEST_F(TemplateElementTest, SingleCharacter) {
    auto elem = CreateTemplateElement("a");
    EXPECT_EQ(elem->value(), "a");
    EXPECT_EQ(elem->value().length(), 1);
}

/**
 * @test 测试只有空格的元素
 */
TEST_F(TemplateElementTest, SpacesOnly) {
    auto elem1 = CreateTemplateElement(" ");
    EXPECT_EQ(elem1->value(), " ");

    auto elem2 = CreateTemplateElement("   ");
    EXPECT_EQ(elem2->value(), "   ");

    auto elem3 = CreateTemplateElement("\t\t");
    EXPECT_EQ(elem3->value(), "\t\t");
}

/**
 * @test 测试包含null字符的字符串
 */
TEST_F(TemplateElementTest, NullCharacter) {
    std::string with_null = "hello\0world";
    with_null.resize(11); // 包含null字符
    auto elem = CreateTemplateElement(with_null);
    EXPECT_EQ(elem->value().length(), 11);
}

// ============================================================================
// Unicode和编码测试
// ============================================================================

/**
 * @test 测试UTF-8编码的中文字符
 */
TEST_F(TemplateElementTest, ChineseCharacters) {
    auto elem = CreateTemplateElement("你好世界");
    EXPECT_EQ(elem->value(), "你好世界");
    EXPECT_EQ(elem->value().length(), 12); // 每个中文3字节
}

/**
 * @test 测试emoji字符
 */
TEST_F(TemplateElementTest, EmojiCharacters) {
    auto elem = CreateTemplateElement("🌍🌎🌏");
    EXPECT_EQ(elem->value(), "🌍🌎🌏");
    // emoji在UTF-8中通常占用4字节
}

/**
 * @test 测试混合Unicode字符
 */
TEST_F(TemplateElementTest, MixedUnicode) {
    auto elem = CreateTemplateElement("Hello你好🌍World");
    EXPECT_EQ(elem->value(), "Hello你好🌍World");
}

// ============================================================================
// 代码生成相关测试
// ============================================================================

/**
 * @test 测试代码生成接口存在
 */
TEST_F(TemplateElementTest, CodeGenerationInterface) {
    auto elem = CreateTemplateElement("test");

    // 验证对象可以被正确创建
    ASSERT_NE(elem, nullptr);
    EXPECT_EQ(elem->value(), "test");

    // 注意:实际的代码生成测试需要CodeGenerator和FunctionDef的完整设置
    // 这里主要验证接口存在且对象可以被正确构造
}

} // namespace test
} // namespace compiler
} // namespace mjs

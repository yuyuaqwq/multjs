#include <gtest/gtest.h>
#include <iostream>
#include "src/compiler/lexer.h"
#include "mjs/error.h"

using namespace mjs;
using namespace mjs::compiler;

/**
 * @brief Lexer 基础功能测试
 */
class LexerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== 基础字符处理测试 ====================

TEST_F(LexerTest, NextChar_ReadsCharacter) {
    std::string source = "abc";
    Lexer lexer(source);

    EXPECT_EQ(lexer.NextToken().type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, PeekChar_LooksAhead) {
    std::string source = "123";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "123");
}

TEST_F(LexerTest, TestString_MatchesPattern) {
    std::string source = "function";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwFunction);
}

TEST_F(LexerTest, TestChar_MatchesSingleCharacter) {
    std::string source = "{";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepLCurly);
}

TEST_F(LexerTest, EmptySource) {
    std::string source = "";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kEof);
}

// ==================== 空白字符和注释处理测试 ====================

TEST_F(LexerTest, SkipWhitespace_Spaces) {
    std::string source = "    let";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, SkipWhitespace_Tabs) {
    std::string source = "\t\t\tlet";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, SkipWhitespace_Newlines) {
    std::string source = "\n\n\nlet";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, SkipWhitespace_Mixed) {
    std::string source = " \t\n\r let";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, SingleLineComment) {
    std::string source = "// This is a comment\nlet";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, SingleLineComment_NoNewline) {
    std::string source = "// This is a comment";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kEof);
}

TEST_F(LexerTest, MultiLineComment) {
    std::string source = "/* This is a\n multi-line comment */ let";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, MultiLineComment_SingleLine) {
    std::string source = "/* comment */ let";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, MultiLineComment_Unclosed_ThrowsException) {
    std::string source = "/* unclosed comment let";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, MixedCommentsAndWhitespace) {
    std::string source = " /* comment 1 */ \n// comment 2\nlet";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

// ==================== 标识符和关键字测试 ====================

TEST_F(LexerTest, Identifier_Simple) {
    std::string source = "variable";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "variable");
}

TEST_F(LexerTest, Identifier_WithUnderscore) {
    std::string source = "_myVar";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "_myVar");
}

TEST_F(LexerTest, Identifier_WithDollar) {
    // 注意：当前实现中，$ 符号不被支持为标识符
    // 这是因为 $ 不在运算符表中，也不被 TryHandleOperator 处理
    // 这个测试文档化了这个限制
    std::string source = "$";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Identifier_WithNumbers) {
    std::string source = "var123";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "var123");
}

TEST_F(LexerTest, Identifier_CannotStartWithNumber) {
    std::string source = "123var";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
}

TEST_F(LexerTest, Keyword_let) {
    std::string source = "let";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, Keyword_const) {
    std::string source = "const";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwConst);
}

TEST_F(LexerTest, Keyword_function) {
    std::string source = "function";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwFunction);
}

TEST_F(LexerTest, Keyword_if) {
    std::string source = "if";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwIf);
}

TEST_F(LexerTest, Keyword_else) {
    std::string source = "else";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwElse);
}

TEST_F(LexerTest, Keyword_while) {
    std::string source = "while";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwWhile);
}

TEST_F(LexerTest, Keyword_for) {
    std::string source = "for";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwFor);
}

TEST_F(LexerTest, Keyword_return) {
    std::string source = "return";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwReturn);
}

TEST_F(LexerTest, Keyword_class) {
    std::string source = "class";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwClass);
}

TEST_F(LexerTest, Keyword_new) {
    std::string source = "new";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwNew);
}

TEST_F(LexerTest, Keyword_this) {
    std::string source = "this";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwThis);
}

TEST_F(LexerTest, Keyword_super) {
    std::string source = "super";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwSuper);
}

TEST_F(LexerTest, Keyword_extends) {
    std::string source = "extends";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwExtends);
}

TEST_F(LexerTest, Keyword_static) {
    std::string source = "static";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwStatic);
}

TEST_F(LexerTest, Keyword_import) {
    std::string source = "import";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwImport);
}

TEST_F(LexerTest, Keyword_export) {
    std::string source = "export";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwExport);
}

TEST_F(LexerTest, Keyword_from) {
    std::string source = "from";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwFrom);
}

TEST_F(LexerTest, Keyword_as) {
    std::string source = "as";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwAs);
}

TEST_F(LexerTest, Keyword_async) {
    std::string source = "async";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwAsync);
}

TEST_F(LexerTest, Keyword_await) {
    std::string source = "await";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwAwait);
}

TEST_F(LexerTest, Keyword_yield) {
    std::string source = "yield";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwYield);
}

TEST_F(LexerTest, Keyword_try) {
    std::string source = "try";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwTry);
}

TEST_F(LexerTest, Keyword_catch) {
    std::string source = "catch";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwCatch);
}

TEST_F(LexerTest, Keyword_finally) {
    std::string source = "finally";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwFinally);
}

TEST_F(LexerTest, Keyword_throw) {
    std::string source = "throw";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwThrow);
}

TEST_F(LexerTest, Keyword_typeof) {
    std::string source = "typeof";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwTypeof);
}

TEST_F(LexerTest, Keyword_instanceof) {
    std::string source = "instanceof";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwInstanceof);
}

TEST_F(LexerTest, Keyword_void) {
    std::string source = "void";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwVoid);
}

TEST_F(LexerTest, Keyword_delete) {
    std::string source = "delete";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwDelete);
}

TEST_F(LexerTest, Keyword_in) {
    std::string source = "in";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwIn);
}

TEST_F(LexerTest, Keyword_with) {
    std::string source = "with";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwWith);
}

TEST_F(LexerTest, Keyword_switch) {
    std::string source = "switch";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwSwitch);
}

TEST_F(LexerTest, Keyword_case) {
    std::string source = "case";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwCase);
}

TEST_F(LexerTest, Keyword_default) {
    std::string source = "default";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwDefault);
}

TEST_F(LexerTest, Keyword_break) {
    std::string source = "break";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwBreak);
}

TEST_F(LexerTest, Keyword_continue) {
    std::string source = "continue";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwContinue);
}

TEST_F(LexerTest, Keyword_get) {
    std::string source = "get";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwGet);
}

TEST_F(LexerTest, Keyword_set) {
    std::string source = "set";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwSet);
}

TEST_F(LexerTest, ReservedWord_NaN) {
    std::string source = "NaN";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "NaN");
}

TEST_F(LexerTest, ReservedWord_Infinity) {
    std::string source = "Infinity";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "Infinity");
}

// ==================== 数字字面量测试 ====================

TEST_F(LexerTest, Integer_Simple) {
    std::string source = "123";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "123");
}

TEST_F(LexerTest, Integer_Zero) {
    std::string source = "0";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0");
}

TEST_F(LexerTest, Integer_WithSeparator) {
    std::string source = "1_000_000";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    // 注意：数字分隔符 _ 会被跳过，不包含在值中
    EXPECT_EQ(token.value(), "1000000");
}

TEST_F(LexerTest, Float_Simple) {
    std::string source = "3.14";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "3.14");
}

TEST_F(LexerTest, Float_NoIntegerPart) {
    std::string source = "0.5";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "0.5");
}

TEST_F(LexerTest, Float_NoFractionPart) {
    std::string source = "42.";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "42.");
}

TEST_F(LexerTest, Float_ScientificNotation_Lowercase) {
    std::string source = "1.5e10";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "1.5e10");
}

TEST_F(LexerTest, Float_ScientificNotation_Uppercase) {
    std::string source = "1.5E10";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "1.5E10");
}

TEST_F(LexerTest, Float_ScientificNotation_Positive) {
    std::string source = "1.5e+10";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "1.5e+10");
}

TEST_F(LexerTest, Float_ScientificNotation_Negative) {
    std::string source = "1.5e-10";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "1.5e-10");
}

TEST_F(LexerTest, Float_ScientificNotation_IntegerBase) {
    std::string source = "2e10";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kFloat);
    EXPECT_EQ(token.value(), "2e10");
}

TEST_F(LexerTest, Float_ScientificNotation_NoExponent_ThrowsException) {
    std::string source = "1e";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Hexadecimal_Simple) {
    std::string source = "0xFF";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0xFF");
}

TEST_F(LexerTest, Hexadecimal_Lowercase) {
    std::string source = "0xff";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0xff");
}

TEST_F(LexerTest, Hexadecimal_WithSeparator) {
    std::string source = "0xFF_FF";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    // 分隔符被跳过
    EXPECT_EQ(token.value(), "0xFFFF");
}

TEST_F(LexerTest, Hexadecimal_BigInt) {
    std::string source = "0xFFn";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kBigInt);
    // BigInt 后缀 n 会被跳过，不包含在值中
    EXPECT_EQ(token.value(), "0xFF");
}

TEST_F(LexerTest, Hexadecimal_NoDigits_ThrowsException) {
    std::string source = "0x";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Hexadecimal_InvalidDigit_ThrowsException) {
    std::string source = "0xGH";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Binary_Simple) {
    std::string source = "0b1010";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0b1010");
}

TEST_F(LexerTest, Binary_Uppercase) {
    std::string source = "0B1010";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0B1010");
}

TEST_F(LexerTest, Binary_WithSeparator) {
    std::string source = "0b1010_1100";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    // 分隔符被跳过
    EXPECT_EQ(token.value(), "0b10101100");
}

TEST_F(LexerTest, Binary_BigInt) {
    std::string source = "0b1010n";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kBigInt);
    // BigInt 后缀 n 会被跳过
    EXPECT_EQ(token.value(), "0b1010");
}

TEST_F(LexerTest, Binary_NoDigits_ThrowsException) {
    std::string source = "0b";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Binary_InvalidDigit_ThrowsException) {
    std::string source = "0b102";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Octal_Simple) {
    std::string source = "0o755";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0o755");
}

TEST_F(LexerTest, Octal_Uppercase) {
    std::string source = "0O755";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0O755");
}

TEST_F(LexerTest, Octal_WithSeparator) {
    std::string source = "0o755_644";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    // 分隔符被跳过
    EXPECT_EQ(token.value(), "0o755644");
}

TEST_F(LexerTest, Octal_BigInt) {
    std::string source = "0o755n";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kBigInt);
    // BigInt 后缀 n 会被跳过
    EXPECT_EQ(token.value(), "0o755");
}

TEST_F(LexerTest, Octal_NoDigits_ThrowsException) {
    std::string source = "0o";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, Octal_InvalidDigit_ThrowsException) {
    std::string source = "0o789";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, BigInt_Simple) {
    std::string source = "123n";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kBigInt);
    // BigInt 后缀 n 会被跳过
    EXPECT_EQ(token.value(), "123");
}

TEST_F(LexerTest, BigInt_WithSeparator) {
    std::string source = "1_000_000n";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kBigInt);
    // 分隔符和 BigInt 后缀都会被跳过
    EXPECT_EQ(token.value(), "1000000");
}

TEST_F(LexerTest, BigInt_WithFloat_ThrowsException) {
    std::string source = "123.45n";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, BigInt_WithExponent_ThrowsException) {
    std::string source = "123e10n";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, ZeroPrefixedNumber_JustZero) {
    std::string source = "0";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0");
}

TEST_F(LexerTest, ZeroPrefixedNumber_DecimalAfterZero) {
    std::string source = "0123";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kInteger);
    EXPECT_EQ(token.value(), "0123");
}

// ==================== 字符串字面量测试 ====================

TEST_F(LexerTest, String_SingleQuotes) {
    std::string source = "'hello'";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "hello");
}

TEST_F(LexerTest, String_DoubleQuotes) {
    std::string source = "\"world\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "world");
}

TEST_F(LexerTest, String_Empty) {
    std::string source = "\"\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "");
}

TEST_F(LexerTest, String_Escape_Backslash) {
    std::string source = "\"\\\\\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\\");
}

TEST_F(LexerTest, String_Escape_Newline) {
    std::string source = "\"\\n\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\n");
}

TEST_F(LexerTest, String_Escape_Tab) {
    std::string source = "\"\\t\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\t");
}

TEST_F(LexerTest, String_Escape_CarriageReturn) {
    std::string source = "\"\\r\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\r");
}

TEST_F(LexerTest, String_Escape_Backspace) {
    std::string source = "\"\\b\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\b");
}

TEST_F(LexerTest, String_Escape_FormFeed) {
    std::string source = "\"\\f\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\f");
}

TEST_F(LexerTest, String_Escape_VerticalTab) {
    std::string source = "\"\\v\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\v");
}

TEST_F(LexerTest, String_Escape_Quote) {
    std::string source = "\"\\\"\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "\"");
}

TEST_F(LexerTest, String_Escape_SingleQuote) {
    std::string source = "'\\''";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "'");
}

TEST_F(LexerTest, String_Escape_Backtick) {
    std::string source = "\"\\`\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "`");
}

TEST_F(LexerTest, String_Escape_LineContinuation) {
    std::string source = "\"hello\\\nworld\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "helloworld");
}

TEST_F(LexerTest, String_Escape_Hexadecimal) {
    std::string source = "\"\\x41\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "A");
}

TEST_F(LexerTest, String_Escape_Hexadecimal_TwoBytes) {
    std::string source = "\"\\x41\\x42\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "AB");
}

TEST_F(LexerTest, String_Escape_Hexadecimal_Incomplete_ThrowsException) {
    std::string source = "\"\\x4\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Hexadecimal_Invalid_ThrowsException) {
    std::string source = "\"\\xGH\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Unicode_Simple) {
    std::string source = "\"\\u0041\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "A");
}

TEST_F(LexerTest, String_Escape_Unicode_Chinese) {
    std::string source = "\"\\u4E2D\\u6587\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value(), "中文");
}

TEST_F(LexerTest, String_Escape_Unicode_Incomplete_ThrowsException) {
    std::string source = "\"\\u041\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Unicode_Extended) {
    std::string source = "\"\\u{1F600}\"";  // 😃
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    // Emoji 编码为 UTF-8
    EXPECT_EQ(token.value().size(), 4);  // UTF-8 编码的 emoji 占 4 字节
}

TEST_F(LexerTest, String_Escape_Unicode_Extended_Max) {
    std::string source = "\"\\u{10FFFF}\"";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
}

TEST_F(LexerTest, String_Escape_Unicode_Extended_TooLarge_ThrowsException) {
    std::string source = "\"\\u{110000}\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Unicode_Extended_Empty_ThrowsException) {
    std::string source = "\"\\u{}\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Unicode_SurrogatePair) {
    std::string source = "\"\\uD83D\\uDE00\"";  // 😃 的代理对
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kString);
    EXPECT_EQ(token.value().size(), 4);  // UTF-8 编码
}

TEST_F(LexerTest, String_Escape_Unicode_SurrogatePair_LowOnly_ThrowsException) {
    std::string source = "\"\\uDE00\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Unicode_SurrogatePair_Incomplete_ThrowsException) {
    std::string source = "\"\\uD83D\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Escape_Invalid_ThrowsException) {
    std::string source = "\"\\z\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_Unclosed_ThrowsException) {
    std::string source = "\"hello";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, String_UnescapedNewline_ThrowsException) {
    std::string source = "\"hello\nworld\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

// ==================== 模板字符串测试 ====================

TEST_F(LexerTest, TemplateString_Simple) {
    std::string source = "`hello`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kTemplateElement);
    EXPECT_EQ(token2.value(), "hello");

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kBacktick);
}

TEST_F(LexerTest, TemplateString_Empty) {
    std::string source = "``";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    // 空模板字符串立即结束，所以下一个 token 是 kBacktick 而不是 kTemplateElement
    EXPECT_EQ(token2.type(), TokenType::kBacktick);
}

TEST_F(LexerTest, TemplateString_WithInterpolation) {
    std::string source = "`hello ${name}`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kTemplateElement);
    EXPECT_EQ(token2.value(), "hello ");

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kTemplateInterpolationStart);

    auto token4 = lexer.NextToken();
    EXPECT_EQ(token4.type(), TokenType::kIdentifier);
    EXPECT_EQ(token4.value(), "name");

    auto token5 = lexer.NextToken();
    EXPECT_EQ(token5.type(), TokenType::kTemplateInterpolationEnd);

    auto token6 = lexer.NextToken();
    EXPECT_EQ(token6.type(), TokenType::kBacktick);
}

TEST_F(LexerTest, TemplateString_MultipleInterpolations) {
    std::string source = "`${a} ${b}`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    // 空字符串被跳过，直接到插值开始
    EXPECT_EQ(token2.type(), TokenType::kTemplateInterpolationStart);

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kIdentifier);
    EXPECT_EQ(token3.value(), "a");

    auto token4 = lexer.NextToken();
    EXPECT_EQ(token4.type(), TokenType::kTemplateInterpolationEnd);

    auto token5 = lexer.NextToken();
    EXPECT_EQ(token5.type(), TokenType::kTemplateElement);
    EXPECT_EQ(token5.value(), " ");

    auto token6 = lexer.NextToken();
    EXPECT_EQ(token6.type(), TokenType::kTemplateInterpolationStart);

    auto token7 = lexer.NextToken();
    EXPECT_EQ(token7.type(), TokenType::kIdentifier);
    EXPECT_EQ(token7.value(), "b");

    auto token8 = lexer.NextToken();
    EXPECT_EQ(token8.type(), TokenType::kTemplateInterpolationEnd);

    auto token9 = lexer.NextToken();
    EXPECT_EQ(token9.type(), TokenType::kBacktick);
}

TEST_F(LexerTest, TemplateString_Nested) {
    std::string source = "`outer ${a + `inner ${b}`}`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kTemplateElement);

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kTemplateInterpolationStart);

    auto token4 = lexer.NextToken();
    EXPECT_EQ(token4.type(), TokenType::kIdentifier);

    auto token5 = lexer.NextToken();
    EXPECT_EQ(token5.type(), TokenType::kOpAdd);

    auto token6 = lexer.NextToken();
    EXPECT_EQ(token6.type(), TokenType::kBacktick);  // 嵌套模板开始

    auto token7 = lexer.NextToken();
    EXPECT_EQ(token7.type(), TokenType::kTemplateElement);

    auto token8 = lexer.NextToken();
    EXPECT_EQ(token8.type(), TokenType::kTemplateInterpolationStart);

    auto token9 = lexer.NextToken();
    EXPECT_EQ(token9.type(), TokenType::kIdentifier);

    auto token10 = lexer.NextToken();
    EXPECT_EQ(token10.type(), TokenType::kTemplateInterpolationEnd);

    auto token11 = lexer.NextToken();
    EXPECT_EQ(token11.type(), TokenType::kBacktick);  // 嵌套模板结束

    auto token12 = lexer.NextToken();
    EXPECT_EQ(token12.type(), TokenType::kTemplateInterpolationEnd);

    auto token13 = lexer.NextToken();
    EXPECT_EQ(token13.type(), TokenType::kBacktick);  // 外层模板结束
}

TEST_F(LexerTest, TemplateString_WithNewlines) {
    std::string source = "`line1\nline2\nline3`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kTemplateElement);
    EXPECT_EQ(token2.value(), "line1\nline2\nline3");

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kBacktick);
}

TEST_F(LexerTest, TemplateString_WithEscapeSequences) {
    std::string source = "`\\n\\t\\r`";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kBacktick);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kTemplateElement);
    EXPECT_EQ(token2.value(), "\n\t\r");

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kBacktick);
}

// ==================== 正则表达式字面量测试 ====================

TEST_F(LexerTest, RegExp_Simple) {
    std::string source = "/abc/";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "abc");
    EXPECT_EQ(token.regex_flags(), "");
}

TEST_F(LexerTest, RegExp_WithFlags) {
    std::string source = "/abc/gim";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "abc");
    EXPECT_EQ(token.regex_flags(), "gim");
}

TEST_F(LexerTest, RegExp_AllFlags) {
    std::string source = "/abc/gimsuyd";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "abc");
    EXPECT_EQ(token.regex_flags(), "gimsuyd");
}

TEST_F(LexerTest, RegExp_WithEscape) {
    std::string source = "/\\d+/";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "\\d+");
}

TEST_F(LexerTest, RegExp_WithCharacterClass) {
    std::string source = "/[a-z]/";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "[a-z]");
}

TEST_F(LexerTest, RegExp_WithNestedCharacterClass) {
    std::string source = "/[a-z[0-9]]/";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "[a-z[0-9]]");
}

TEST_F(LexerTest, RegExp_Complex) {
    std::string source = "/\\b\\w+\\b/g";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kRegExp);
    EXPECT_EQ(token.value(), "\\b\\w+\\b");
    EXPECT_EQ(token.regex_flags(), "g");
}

TEST_F(LexerTest, RegExp_AfterOperator) {
    std::string source = "a = /abc/";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpAssign);

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kRegExp);
}

TEST_F(LexerTest, RegExp_NotAfterIdentifier) {
    std::string source = "a/abc";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);
    EXPECT_EQ(token1.value(), "a");

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpDiv);
}

TEST_F(LexerTest, RegExp_NotAfterNumber) {
    std::string source = "123/abc";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kInteger);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpDiv);
}

TEST_F(LexerTest, RegExp_AfterLeftParen) {
    std::string source = "(/abc/)";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kSepLParen);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kRegExp);
}

TEST_F(LexerTest, RegExp_AfterLeftBrace) {
    std::string source = "{/abc/}";
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kSepLCurly);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kRegExp);
}

TEST_F(LexerTest, RegExp_Unterminated_ThrowsException) {
    std::string source = "/abc";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, RegExp_UnterminatedWithNewline_ThrowsException) {
    std::string source = "/abc\n";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

// ==================== 运算符测试 ====================

TEST_F(LexerTest, Operator_Assign) {
    std::string source = "=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpAssign);
}

TEST_F(LexerTest, Operator_Add) {
    std::string source = "+";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpAdd);
}

TEST_F(LexerTest, Operator_Sub) {
    std::string source = "-";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpSub);
}

TEST_F(LexerTest, Operator_Mul) {
    std::string source = "*";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpMul);
}

TEST_F(LexerTest, Operator_Div) {
    std::string source = "a / b";  // 需要在有上下文的情况下才能识别为除法
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpDiv);

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, Operator_Mod) {
    std::string source = "%";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpMod);
}

TEST_F(LexerTest, Operator_Power) {
    std::string source = "**";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpPower);
}

TEST_F(LexerTest, Operator_Inc) {
    std::string source = "++";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpInc);
}

TEST_F(LexerTest, Operator_Dec) {
    std::string source = "--";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpDec);
}

TEST_F(LexerTest, Operator_AddAssign) {
    std::string source = "+=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpAddAssign);
}

TEST_F(LexerTest, Operator_SubAssign) {
    std::string source = "-=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpSubAssign);
}

TEST_F(LexerTest, Operator_MulAssign) {
    std::string source = "*=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpMulAssign);
}

TEST_F(LexerTest, Operator_DivAssign) {
    std::string source = "a /= b";  // 需要在有上下文的情况下才能识别为除法赋值
    Lexer lexer(source);

    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpDivAssign);

    auto token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, Operator_ModAssign) {
    std::string source = "%=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpModAssign);
}

TEST_F(LexerTest, Operator_PowerAssign) {
    std::string source = "**=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpPowerAssign);
}

TEST_F(LexerTest, Operator_BitAndAssign) {
    std::string source = "&=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitAndAssign);
}

TEST_F(LexerTest, Operator_BitOrAssign) {
    std::string source = "|=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitOrAssign);
}

TEST_F(LexerTest, Operator_BitXorAssign) {
    std::string source = "^=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitXorAssign);
}

TEST_F(LexerTest, Operator_ShiftLeftAssign) {
    std::string source = "<<=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpShiftLeftAssign);
}

TEST_F(LexerTest, Operator_ShiftRightAssign) {
    std::string source = ">>=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpShiftRightAssign);
}

TEST_F(LexerTest, Operator_UnsignedShiftRightAssign) {
    std::string source = ">>>=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpUnsignedShiftRightAssign);
}

TEST_F(LexerTest, Operator_BitNot) {
    std::string source = "~";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitNot);
}

TEST_F(LexerTest, Operator_BitAnd) {
    std::string source = "&";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitAnd);
}

TEST_F(LexerTest, Operator_BitOr) {
    std::string source = "|";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitOr);
}

TEST_F(LexerTest, Operator_BitXor) {
    std::string source = "^";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpBitXor);
}

TEST_F(LexerTest, Operator_ShiftLeft) {
    std::string source = "<<";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpShiftLeft);
}

TEST_F(LexerTest, Operator_ShiftRight) {
    std::string source = ">>";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpShiftRight);
}

TEST_F(LexerTest, Operator_UnsignedShiftRight) {
    std::string source = ">>>";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpUnsignedShiftRight);
}

TEST_F(LexerTest, Operator_Not) {
    std::string source = "!";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpNot);
}

TEST_F(LexerTest, Operator_And) {
    std::string source = "&&";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpAnd);
}

TEST_F(LexerTest, Operator_Or) {
    std::string source = "||";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpOr);
}

TEST_F(LexerTest, Operator_Eq) {
    std::string source = "==";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpEq);
}

TEST_F(LexerTest, Operator_Ne) {
    std::string source = "!=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpNe);
}

TEST_F(LexerTest, Operator_StrictEq) {
    std::string source = "===";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpStrictEq);
}

TEST_F(LexerTest, Operator_StrictNe) {
    std::string source = "!==";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpStrictNe);
}

TEST_F(LexerTest, Operator_Lt) {
    std::string source = "<";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpLt);
}

TEST_F(LexerTest, Operator_Le) {
    std::string source = "<=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpLe);
}

TEST_F(LexerTest, Operator_Gt) {
    std::string source = ">";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpGt);
}

TEST_F(LexerTest, Operator_Ge) {
    std::string source = ">=";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpGe);
}

TEST_F(LexerTest, Operator_NullishCoalescing) {
    std::string source = "??";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpNullishCoalescing);
}

TEST_F(LexerTest, Operator_OptionalChain) {
    std::string source = "?.";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpOptionalChain);
}

TEST_F(LexerTest, Separator_Semi) {
    std::string source = ";";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepSemi);
}

TEST_F(LexerTest, Separator_Comma) {
    std::string source = ",";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepComma);
}

TEST_F(LexerTest, Separator_Dot) {
    std::string source = ".";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepDot);
}

TEST_F(LexerTest, Separator_Ellipsis) {
    std::string source = "...";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepEllipsis);
}

TEST_F(LexerTest, Separator_Colon) {
    std::string source = ":";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepColon);
}

TEST_F(LexerTest, Separator_Question) {
    std::string source = "?";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepQuestion);
}

TEST_F(LexerTest, Separator_Arrow) {
    std::string source = "=>";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepArrow);
}

TEST_F(LexerTest, Separator_LParen) {
    std::string source = "(";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepLParen);
}

TEST_F(LexerTest, Separator_RParen) {
    std::string source = ")";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepRParen);
}

TEST_F(LexerTest, Separator_LBrack) {
    std::string source = "[";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepLBrack);
}

TEST_F(LexerTest, Separator_RBrack) {
    std::string source = "]";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepRBrack);
}

TEST_F(LexerTest, Separator_LCurly) {
    std::string source = "{";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepLCurly);
}

TEST_F(LexerTest, Separator_RCurly) {
    std::string source = "}";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kSepRCurly);
}

TEST_F(LexerTest, Operator_Ternary) {
    std::string source = "?:";
    Lexer lexer(source);

    // 在 JavaScript 中，?: 不是单个运算符，而是两个独立的运算符
    auto token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kSepQuestion);

    auto token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kSepColon);
}

// ==================== Token 预览和匹配测试 ====================

TEST_F(LexerTest, PeekToken_LooksAhead) {
    std::string source = "let x";
    Lexer lexer(source);

    auto peek_token = lexer.PeekToken();
    EXPECT_EQ(peek_token.type(), TokenType::kKwLet);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, PeekToken_CalledMultipleTimes) {
    std::string source = "let x";
    Lexer lexer(source);

    auto peek1 = lexer.PeekToken();
    auto peek2 = lexer.PeekToken();

    EXPECT_EQ(peek1.type(), TokenType::kKwLet);
    EXPECT_EQ(peek2.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, PeekTokenN_SecondToken) {
    std::string source = "let x";
    Lexer lexer(source);

    auto token = lexer.PeekTokenN(2);
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "x");
}

TEST_F(LexerTest, PeekTokenN_ThirdToken) {
    std::string source = "let x =";
    Lexer lexer(source);

    auto token = lexer.PeekTokenN(3);
    EXPECT_EQ(token.type(), TokenType::kOpAssign);
}

TEST_F(LexerTest, PeekTokenN_DoesNotConsume) {
    std::string source = "let x =";
    Lexer lexer(source);

    auto peek1 = lexer.PeekTokenN(1);
    EXPECT_EQ(peek1.type(), TokenType::kKwLet);

    auto peek3 = lexer.PeekTokenN(3);
    EXPECT_EQ(peek3.type(), TokenType::kOpAssign);

    auto next = lexer.NextToken();
    EXPECT_EQ(next.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, PeekTokenN_Zero_ThrowsException) {
    std::string source = "let";
    Lexer lexer(source);

    EXPECT_THROW(lexer.PeekTokenN(0), std::invalid_argument);
}

TEST_F(LexerTest, MatchToken_Success) {
    std::string source = "let";
    Lexer lexer(source);

    auto token = lexer.MatchToken(TokenType::kKwLet);
    EXPECT_EQ(token.type(), TokenType::kKwLet);
}

TEST_F(LexerTest, MatchToken_Failure_ThrowsException) {
    std::string source = "let";
    Lexer lexer(source);

    EXPECT_THROW(lexer.MatchToken(TokenType::kKwIf), SyntaxError);
}

// ==================== 检查点和回溯测试 ====================

TEST_F(LexerTest, CreateCheckpoint_SavesState) {
    std::string source = "let x = 42";
    Lexer lexer(source);

    lexer.NextToken();  // 消耗 'let'
    auto checkpoint = lexer.CreateCheckpoint();

    lexer.NextToken();  // 消耗 'x'
    lexer.NextToken();  // 消耗 '='

    lexer.RewindToCheckpoint(checkpoint);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "x");
}

TEST_F(LexerTest, RewindToCheckpoint_RestoresPosition) {
    std::string source = "let x = 42";
    Lexer lexer(source);

    auto checkpoint1 = lexer.CreateCheckpoint();

    lexer.NextToken();  // 'let'
    lexer.NextToken();  // 'x'

    auto checkpoint2 = lexer.CreateCheckpoint();

    lexer.NextToken();  // '='
    lexer.NextToken();  // '42'

    lexer.RewindToCheckpoint(checkpoint2);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kOpAssign);
}

TEST_F(LexerTest, RewindToCheckpoint_TemplateState) {
    std::string source = "`hello ${name}`";
    Lexer lexer(source);

    lexer.NextToken();  // kBacktick
    lexer.NextToken();  // kTemplateElement
    lexer.NextToken();  // kTemplateInterpolationStart

    auto checkpoint = lexer.CreateCheckpoint();

    lexer.NextToken();  // 'name'
    lexer.NextToken();  // kTemplateInterpolationEnd

    lexer.RewindToCheckpoint(checkpoint);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.type(), TokenType::kIdentifier);
    EXPECT_EQ(token.value(), "name");
}

// ==================== 复杂场景测试 ====================

TEST_F(LexerTest, ComplexExpression) {
    std::string source = "let x = a + b * c";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kKwLet);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "x");

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kOpAssign);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kIdentifier);
    EXPECT_EQ(t4.value(), "a");

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kOpAdd);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kIdentifier);
    EXPECT_EQ(t6.value(), "b");

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kOpMul);

    auto t8 = lexer.NextToken();
    EXPECT_EQ(t8.type(), TokenType::kIdentifier);
    EXPECT_EQ(t8.value(), "c");
}

TEST_F(LexerTest, FunctionDeclaration) {
    std::string source = "function add(a, b) { return a + b; }";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kKwFunction);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "add");

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kSepLParen);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kIdentifier);
    EXPECT_EQ(t4.value(), "a");

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kSepComma);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kIdentifier);
    EXPECT_EQ(t6.value(), "b");

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kSepRParen);

    auto t8 = lexer.NextToken();
    EXPECT_EQ(t8.type(), TokenType::kSepLCurly);

    auto t9 = lexer.NextToken();
    EXPECT_EQ(t9.type(), TokenType::kKwReturn);

    auto t10 = lexer.NextToken();
    EXPECT_EQ(t10.type(), TokenType::kIdentifier);
    EXPECT_EQ(t10.value(), "a");

    auto t11 = lexer.NextToken();
    EXPECT_EQ(t11.type(), TokenType::kOpAdd);

    auto t12 = lexer.NextToken();
    EXPECT_EQ(t12.type(), TokenType::kIdentifier);
    EXPECT_EQ(t12.value(), "b");

    auto t13 = lexer.NextToken();
    EXPECT_EQ(t13.type(), TokenType::kSepSemi);

    auto t14 = lexer.NextToken();
    EXPECT_EQ(t14.type(), TokenType::kSepRCurly);
}

TEST_F(LexerTest, ArrowFunction) {
    std::string source = "(a, b) => a + b";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kSepLParen);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kSepComma);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kIdentifier);

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kSepRParen);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kSepArrow);

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, ArrayLiteral) {
    std::string source = "[1, 2, 3]";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kSepLBrack);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kInteger);

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kSepComma);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kInteger);

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kSepComma);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kInteger);

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kSepRBrack);
}

TEST_F(LexerTest, ObjectLiteral) {
    std::string source = "{ x: 1, y: 2 }";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kSepLCurly);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "x");

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kSepColon);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kInteger);

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kSepComma);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kIdentifier);
    EXPECT_EQ(t6.value(), "y");

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kSepColon);

    auto t8 = lexer.NextToken();
    EXPECT_EQ(t8.type(), TokenType::kInteger);

    auto t9 = lexer.NextToken();
    EXPECT_EQ(t9.type(), TokenType::kSepRCurly);
}

TEST_F(LexerTest, ClassDeclaration) {
    std::string source = "class Animal { }";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kKwClass);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "Animal");

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kSepLCurly);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kSepRCurly);
}

TEST_F(LexerTest, ClassExtends) {
    std::string source = "class Dog extends Animal { }";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kKwClass);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "Dog");

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kKwExtends);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kIdentifier);
    EXPECT_EQ(t4.value(), "Animal");
}

TEST_F(LexerTest, TernaryOperator) {
    std::string source = "a > b ? a : b";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kIdentifier);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kOpGt);

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kIdentifier);

    auto t4 = lexer.NextToken();
    EXPECT_EQ(t4.type(), TokenType::kSepQuestion);

    auto t5 = lexer.NextToken();
    EXPECT_EQ(t5.type(), TokenType::kIdentifier);

    auto t6 = lexer.NextToken();
    EXPECT_EQ(t6.type(), TokenType::kSepColon);

    auto t7 = lexer.NextToken();
    EXPECT_EQ(t7.type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, OptionalChaining) {
    std::string source = "obj?.prop";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kIdentifier);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kOpOptionalChain);

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kIdentifier);
    EXPECT_EQ(t3.value(), "prop");
}

TEST_F(LexerTest, NullishCoalescing) {
    std::string source = "a ?? b";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kIdentifier);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kOpNullishCoalescing);

    auto t3 = lexer.NextToken();
    EXPECT_EQ(t3.type(), TokenType::kIdentifier);
}

TEST_F(LexerTest, SpreadOperator) {
    std::string source = "...args";
    Lexer lexer(source);

    auto t1 = lexer.NextToken();
    EXPECT_EQ(t1.type(), TokenType::kSepEllipsis);

    auto t2 = lexer.NextToken();
    EXPECT_EQ(t2.type(), TokenType::kIdentifier);
    EXPECT_EQ(t2.value(), "args");
}

// ==================== 位置信息测试 ====================

TEST_F(LexerTest, GetSourcePosition_SkipsWhitespace) {
    std::string source = "   let";
    Lexer lexer(source);

    auto pos = lexer.GetSourcePosition();
    EXPECT_EQ(pos, 3);  // 跳过3个空格后的位置
}

TEST_F(LexerTest, GetRawSourcePosition_NoSkip) {
    std::string source = "   let";
    Lexer lexer(source);

    auto pos = lexer.GetRawSourcePosition();
    EXPECT_EQ(pos, 0);  // 原始位置不跳过空白
}

TEST_F(LexerTest, Token_Position) {
    std::string source = "let x";
    Lexer lexer(source);

    auto token = lexer.NextToken();
    EXPECT_EQ(token.pos(), 0);
}

// ==================== LineTable 测试 ====================

TEST_F(LexerTest, LineTable_BuiltCorrectly) {
    std::string source = "line1\nline2\nline3";
    Lexer lexer(source);

    const auto& line_table = lexer.line_table();
    // LineTable 应该正确构建行号表
    auto [line, col] = line_table.PosToLineAndColumn(0);
    EXPECT_EQ(line, 1);
    EXPECT_EQ(col, 0);
}

// ==================== 错误处理测试 ====================

TEST_F(LexerTest, InvalidCharacter_ThrowsException) {
    std::string source = "@";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidUnicodeEscape_ThrowsException) {
    std::string source = "\"\\u\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidHexEscape_ThrowsException) {
    std::string source = "\"\\x\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, UnclosedString_ThrowsException) {
    std::string source = "\"hello";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidHexNumber_ThrowsException) {
    std::string source = "0x";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidBinaryNumber_ThrowsException) {
    std::string source = "0b";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidOctalNumber_ThrowsException) {
    std::string source = "0o";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, BigIntWithDecimal_ThrowsException) {
    std::string source = "123.45n";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, BigIntWithExponent_ThrowsException) {
    std::string source = "123e10n";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidExponent_ThrowsException) {
    std::string source = "123e";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidBinaryDigit_ThrowsException) {
    std::string source = "0b102";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidOctalDigit_ThrowsException) {
    std::string source = "0o789";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidHexDigit_ThrowsException) {
    std::string source = "0xGH";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, UnterminatedRegExp_ThrowsException) {
    std::string source = "/abc";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, UnterminatedMultiLineComment_ThrowsException) {
    std::string source = "/* comment";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, LoneUnicodeLowSurrogate_ThrowsException) {
    std::string source = "\"\\uDE00\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, IncompleteUnicodeSurrogatePair_ThrowsException) {
    std::string source = "\"\\uD83D\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, UnicodeCodePointOutOfRange_ThrowsException) {
    std::string source = "\"\\u{110000}\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, EmptyUnicodeCodePoint_ThrowsException) {
    std::string source = "\"\\u{}\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, InvalidEscapeSequence_ThrowsException) {
    std::string source = "\"\\z\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, IncompleteEscapeSequence_ThrowsException) {
    std::string source = "\"\\";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

TEST_F(LexerTest, UnclosedStringWithNewline_ThrowsException) {
    std::string source = "\"hello\nworld\"";
    Lexer lexer(source);

    EXPECT_THROW(lexer.NextToken(), SyntaxError);
}

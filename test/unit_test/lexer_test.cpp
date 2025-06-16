#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <mjs/error.h>

#include "../src/compiler/lexer.h"

namespace mjs {
namespace compiler {
namespace test {

// 辅助函数：将词法分析器的所有标记收集到向量中
std::vector<Token> CollectAllTokens(Lexer& lexer) {
    std::vector<Token> tokens;
    
    Token token;
    do {
        token = lexer.NextToken();
        tokens.push_back(token);
    } while (!token.is(TokenType::kEof));
    
    return tokens;
}

// 基本标识符和关键字测试
TEST(LexerTest, IdentifiersAndKeywords) {
    Lexer lexer("let x = 5; const y = true; function test() { return x + y; }");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_GE(tokens.size(), 16);
    
    EXPECT_EQ(tokens[0].type(), TokenType::kKwLet);
    EXPECT_EQ(tokens[1].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[1].value(), "x");
    EXPECT_EQ(tokens[2].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[3].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].value(), "5");
    EXPECT_EQ(tokens[4].type(), TokenType::kSepSemi);
    EXPECT_EQ(tokens[5].type(), TokenType::kKwConst);
    EXPECT_EQ(tokens[6].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[6].value(), "y");
    EXPECT_EQ(tokens[7].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[8].type(), TokenType::kTrue);
    EXPECT_EQ(tokens[9].type(), TokenType::kSepSemi);
    EXPECT_EQ(tokens[10].type(), TokenType::kKwFunction);
}

// 数字字面量测试
TEST(LexerTest, NumberLiterals) {
    Lexer lexer("123 0xFF 0b1010 0o777 3.14 1e10 1.5e-5");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 8); // 7个数字 + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[0].value(), "123");
    
    EXPECT_EQ(tokens[1].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[1].value(), "0xFF");
    
    EXPECT_EQ(tokens[2].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[2].value(), "0b1010");
    
    EXPECT_EQ(tokens[3].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].value(), "0o777");
    
    EXPECT_EQ(tokens[4].type(), TokenType::kFloat);
    EXPECT_EQ(tokens[4].value(), "3.14");
    
    EXPECT_EQ(tokens[5].type(), TokenType::kFloat);
    EXPECT_EQ(tokens[5].value(), "1e10");
    
    EXPECT_EQ(tokens[6].type(), TokenType::kFloat);
    EXPECT_EQ(tokens[6].value(), "1.5e-5");
}

// 数字分隔符测试
TEST(LexerTest, NumericSeparators) {
    Lexer lexer("1_000_000 0xFF_FF 0b1010_1010 0o77_77 3.14_15 1e1_0");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 7); // 6个数字 + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[0].value(), "1000000");
    
    EXPECT_EQ(tokens[1].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[1].value(), "0xFFFF");
    
    EXPECT_EQ(tokens[2].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[2].value(), "0b10101010");
    
    EXPECT_EQ(tokens[3].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].value(), "0o7777");
    
    EXPECT_EQ(tokens[4].type(), TokenType::kFloat);
    EXPECT_EQ(tokens[4].value(), "3.1415");
    
    EXPECT_EQ(tokens[5].type(), TokenType::kFloat);
    EXPECT_EQ(tokens[5].value(), "1e10");
}

// BigInt 测试
TEST(LexerTest, BigIntLiterals) {
    Lexer lexer("123n 0xFFn 0b1010n 0o777n");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 5); // 4个BigInt + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kBigInt);
    EXPECT_EQ(tokens[0].value(), "123");
    
    EXPECT_EQ(tokens[1].type(), TokenType::kBigInt);
    EXPECT_EQ(tokens[1].value(), "0xFF");
    
    EXPECT_EQ(tokens[2].type(), TokenType::kBigInt);
    EXPECT_EQ(tokens[2].value(), "0b1010");
    
    EXPECT_EQ(tokens[3].type(), TokenType::kBigInt);
    EXPECT_EQ(tokens[3].value(), "0o777");
}

// 特殊数值测试
TEST(LexerTest, SpecialNumberLiterals) {
    Lexer lexer("NaN Infinity 0");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 4); // NaN + Infinity + 0 + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[0].value(), "NaN");
    
    EXPECT_EQ(tokens[1].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[1].value(), "Infinity");
    
    EXPECT_EQ(tokens[2].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[2].value(), "0");
}

// 字符串字面量测试
TEST(LexerTest, StringLiterals) {
    Lexer lexer("'hello' \"world\" \"escape\\nsequence\" 'quote\\''");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 5); // 4个字符串 + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kString);
    EXPECT_EQ(tokens[0].value(), "hello");
    
    EXPECT_EQ(tokens[1].type(), TokenType::kString);
    EXPECT_EQ(tokens[1].value(), "world");
    
    EXPECT_EQ(tokens[2].type(), TokenType::kString);
    EXPECT_EQ(tokens[2].value(), "escape\nsequence");
    
    EXPECT_EQ(tokens[3].type(), TokenType::kString);
    EXPECT_EQ(tokens[3].value(), "quote'");
}

// Unicode 转义序列测试
TEST(LexerTest, UnicodeEscapeSequences) {
    Lexer lexer("'\\u{1F600}' \"\\u2764\" '\\u{1F4A9}\\u{1F4A5}'");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 4); // 3个字符串 + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kString);
    EXPECT_EQ(tokens[1].type(), TokenType::kString);
    EXPECT_EQ(tokens[2].type(), TokenType::kString);
}

// 运算符和分隔符测试
TEST(LexerTest, OperatorsAndSeparators) {
    Lexer lexer("a + b - c * d / e % f == g != h === i !== j < k <= l > m >= n && o || p");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_GE(tokens.size(), 32); // 16个标识符 + 15个运算符 + EOF
    
    int i = 0;
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // a
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpAdd);      // +
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // b
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpSub);      // -
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // c
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpMul);      // *
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // d
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpDiv);      // /
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // e
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpMod);      // %
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // f
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpEq);       // ==
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // g
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpNe);       // !=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // h
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpStrictEq); // ===
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // i
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpStrictNe); // !==
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // j
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpLt);       // <
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // k
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpLe);       // <=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // l
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpGt);       // >
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // m
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpGe);       // >=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // n
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpAnd);      // &&
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // o
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpOr);       // ||
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // p
}

// 更多运算符测试
TEST(LexerTest, MoreOperators) {
    Lexer lexer("a += b -= c *= d /= e %= f &= g |= h ^= i <<= j >>= k >>>= l");
    
    auto tokens = CollectAllTokens(lexer);
    
    int i = 0;
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // a
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpAddAssign); // +=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // b
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpSubAssign); // -=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // c
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpMulAssign); // *=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // d
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpDivAssign); // /=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // e
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpModAssign); // %=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // f
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpBitAndAssign); // &=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // g
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpBitOrAssign); // |=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // h
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpBitXorAssign); // ^=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // i
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpShiftLeftAssign); // <<=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // j
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpShiftRightAssign); // >>=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // k
    EXPECT_EQ(tokens[i++].type(), TokenType::kOpUnsignedShiftRightAssign); // >>>=
    EXPECT_EQ(tokens[i++].type(), TokenType::kIdentifier); // l
}

// 注释测试
TEST(LexerTest, Comments) {
    Lexer lexer("// 单行注释\nx = 1; /* 多行\n注释 */ y = 2;");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 9); // x = 1; y = 2; + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[0].value(), "x");
    EXPECT_EQ(tokens[1].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[2].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].type(), TokenType::kSepSemi);
    EXPECT_EQ(tokens[4].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[4].value(), "y");
    EXPECT_EQ(tokens[5].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[6].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].type(), TokenType::kSepSemi);
}

// 嵌套注释测试
TEST(LexerTest, NestedComments) {
    Lexer lexer("/* 外层注释 /* 嵌套注释 */ x = 1;");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 5); // x = 1; + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[0].value(), "x");
    EXPECT_EQ(tokens[1].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[2].type(), TokenType::kInteger);
    EXPECT_EQ(tokens[3].type(), TokenType::kSepSemi);
}

// 模板字符串测试
TEST(LexerTest, TemplateStrings) {
    Lexer lexer("`Hello ${name}!`");
    
    auto tokens = CollectAllTokens(lexer);
    
    ASSERT_EQ(tokens.size(), 8); // ` + template + ${ + name + } + ! + ` + EOF
    
    EXPECT_EQ(tokens[0].type(), TokenType::kBacktick);
    EXPECT_EQ(tokens[1].type(), TokenType::kTemplateElement);
    EXPECT_EQ(tokens[1].value(), "Hello ");
    EXPECT_EQ(tokens[2].type(), TokenType::kTemplateInterpolationStart);
    EXPECT_EQ(tokens[3].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[3].value(), "name");
    EXPECT_EQ(tokens[4].type(), TokenType::kTemplateInterpolationEnd);
    EXPECT_EQ(tokens[5].type(), TokenType::kTemplateElement);
    EXPECT_EQ(tokens[5].value(), "!");
    EXPECT_EQ(tokens[6].type(), TokenType::kBacktick);
}

// 复杂模板字符串测试
TEST(LexerTest, ComplexTemplateStrings) {
    Lexer lexer("`Line 1\nLine 2 ${1 + 2} Line 3 ${`Nested ${value}`} End`");
    
    auto tokens = CollectAllTokens(lexer);
    
    // 验证基本结构
    EXPECT_EQ(tokens[0].type(), TokenType::kBacktick);
    EXPECT_EQ(tokens[1].type(), TokenType::kTemplateElement);
    EXPECT_EQ(tokens[1].value(), "Line 1\nLine 2 ");
    EXPECT_EQ(tokens[2].type(), TokenType::kTemplateInterpolationStart);
    // ... 1 + 2 ...
    EXPECT_TRUE(tokens[6].is(TokenType::kTemplateInterpolationEnd));
    EXPECT_EQ(tokens[7].type(), TokenType::kTemplateElement);
    EXPECT_EQ(tokens[7].value(), " Line 3 ");
    EXPECT_EQ(tokens[8].type(), TokenType::kTemplateInterpolationStart);
    EXPECT_EQ(tokens[9].type(), TokenType::kBacktick);
    // ... 嵌套模板 ...
}

// 正则表达式测试
TEST(LexerTest, RegularExpressions) {
    Lexer lexer("let re = /abc/g; let re2 = /[a-z]+/i;");
    
    auto tokens = CollectAllTokens(lexer);
    
    EXPECT_EQ(tokens[0].type(), TokenType::kKwLet);
    EXPECT_EQ(tokens[1].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[1].value(), "re");
    EXPECT_EQ(tokens[2].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[3].type(), TokenType::kRegExp);
    EXPECT_EQ(tokens[3].value(), "abc");
    EXPECT_EQ(tokens[3].regex_flags(), "g");
    
    EXPECT_EQ(tokens[7].type(), TokenType::kIdentifier);
    EXPECT_EQ(tokens[7].value(), "re2");
    EXPECT_EQ(tokens[8].type(), TokenType::kOpAssign);
    EXPECT_EQ(tokens[9].type(), TokenType::kRegExp);
    EXPECT_EQ(tokens[9].value(), "[a-z]+");
    EXPECT_EQ(tokens[9].regex_flags(), "i");
}

// 复杂正则表达式测试
TEST(LexerTest, ComplexRegularExpressions) {
    Lexer lexer("let re = /a\\/b\\[c\\]/gim;");
    
    auto tokens = CollectAllTokens(lexer);
    
    EXPECT_EQ(tokens[3].type(), TokenType::kRegExp);
    EXPECT_EQ(tokens[3].value(), "a\\/b\\[c\\]");
    EXPECT_EQ(tokens[3].regex_flags(), "gim");
}

// 错误处理测试
TEST(LexerTest, ErrorHandling) {
    // 未闭合的字符串
    EXPECT_THROW({
        Lexer lexer("'unclosed string");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 未闭合的多行注释
    EXPECT_THROW({
        Lexer lexer("/* unclosed comment");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 无效的转义序列
    EXPECT_THROW({
        Lexer lexer("'invalid escape \\z'");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 无效的数字格式
    EXPECT_THROW({
        Lexer lexer("0xZZ");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 无效的二进制数字
    EXPECT_THROW({
        Lexer lexer("0b102");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 无效的八进制数字
    EXPECT_THROW({
        Lexer lexer("0o789");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // BigInt不能有小数点
    EXPECT_THROW({
        Lexer lexer("3.14n");
        CollectAllTokens(lexer);
    }, SyntaxError);
    
    // 无效的Unicode转义序列
    EXPECT_THROW({
        Lexer lexer("'\\u{FFFFFF}'"); // 超出范围的Unicode码点
        CollectAllTokens(lexer);
    }, SyntaxError);
}

// PeekToken 和 PeekTokenN 测试
TEST(LexerTest, PeekTokens) {
    Lexer lexer("a + b * c");
    
    // 测试 PeekToken
    Token peek1 = lexer.PeekToken();
    EXPECT_EQ(peek1.type(), TokenType::kIdentifier);
    EXPECT_EQ(peek1.value(), "a");
    
    // 确认 PeekToken 不会消耗标记
    Token token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);
    EXPECT_EQ(token1.value(), "a");
    
    // 测试 PeekTokenN
    Token peek2 = lexer.PeekTokenN(2);
    EXPECT_EQ(peek2.type(), TokenType::kIdentifier);
    EXPECT_EQ(peek2.value(), "b");
    
    // 确认 PeekTokenN 不会消耗标记
    Token token2 = lexer.NextToken();
    EXPECT_EQ(token2.type(), TokenType::kOpAdd);
    
    Token token3 = lexer.NextToken();
    EXPECT_EQ(token3.type(), TokenType::kIdentifier);
    EXPECT_EQ(token3.value(), "b");
}

// 检查点和回溯测试
TEST(LexerTest, CheckpointAndRewind) {
    Lexer lexer("a + b * c");
    
    // 创建检查点
    Lexer::Checkpoint checkpoint = lexer.CreateCheckpoint();
    
    // 消耗一些标记
    lexer.NextToken(); // a
    lexer.NextToken(); // +
    lexer.NextToken(); // b
    
    // 回溯到检查点
    lexer.RewindToCheckpoint(checkpoint);
    
    // 确认回溯成功
    Token token1 = lexer.NextToken();
    EXPECT_EQ(token1.type(), TokenType::kIdentifier);
    EXPECT_EQ(token1.value(), "a");
}

} // namespace test
} // namespace compiler
} // namespace mjs 
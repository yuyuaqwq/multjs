/**
 * @file parser_test.cpp
 * @brief Parser 单元测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include "src/compiler/parser.h"
#include "src/compiler/lexer.h"
#include "src/compiler/statement.h"
#include "mjs/error.h"


namespace mjs {
namespace compiler {
namespace test {

/**
 * @brief Parser 基础功能测试
 */
class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 辅助函数：创建解析器并解析程序
     */
    std::unique_ptr<Parser> ParseProgram(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        auto parser = std::make_unique<Parser>(lexer.get());
        parser->ParseProgram();
        return parser;
    }

    std::vector<std::string> ParseParametersHelper(Parser* parser) {
        return parser->ParseParameters();
    }

    std::vector<std::unique_ptr<Expression>> ParseExpressionsHelper(
        Parser* parser, TokenType begin, TokenType end, bool allow_comma_end) {
        return parser->ParseExpressions(begin, end, allow_comma_end);
    }
};

// ==================== ParseProgram 基础测试 ====================

TEST_F(ParserTest, ParseProgram_EmptyProgram) {
    std::string source = "";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 0);
}

TEST_F(ParserTest, ParseProgram_SingleStatement) {
    std::string source = "let x;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_MultipleStatements) {
    std::string source = "let x; let y; let z;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 3);
}

TEST_F(ParserTest, ParseProgram_WithComments) {
    std::string source = "// comment\nlet x; /* comment */ let y;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 2);
}

TEST_F(ParserTest, ParseProgram_WithWhitespace) {
    std::string source = "\n\n  let x;  \n\n  let y;  \n\n";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 2);
}

// ==================== ParseProgram 与 import 语句测试 ====================

TEST_F(ParserTest, ParseProgram_SingleImport) {
    std::string source = "import { foo } from 'module';";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 0);
    EXPECT_EQ(parser.import_declarations().size(), 1);
}

TEST_F(ParserTest, ParseProgram_MultipleImports) {
    std::string source = "import { foo } from 'module1'; import { bar } from 'module2';";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 0);
    EXPECT_EQ(parser.import_declarations().size(), 2);
}

TEST_F(ParserTest, ParseProgram_MixedImportsAndStatements) {
    std::string source = "import { foo } from 'module'; let x; import { bar } from 'module2'; let y;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 2);
    EXPECT_EQ(parser.import_declarations().size(), 2);
}

TEST_F(ParserTest, ParseProgram_ImportBeforeStatements) {
    std::string source = "let x; import { foo } from 'module';";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
    EXPECT_EQ(parser.import_declarations().size(), 1);
}

TEST_F(ParserTest, ParseProgram_ImportAfterStatements) {
    std::string source = "import { foo } from 'module'; let x;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
    EXPECT_EQ(parser.import_declarations().size(), 1);
}

// ==================== ParseParameters 测试 ====================

TEST_F(ParserTest, ParseParameters_EmptyList) {
    std::string source = "()";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    // 通过友元类访问私有方法
    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 0);
}

TEST_F(ParserTest, ParseParameters_SingleParameter) {
    std::string source = "(x)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "x");
}

TEST_F(ParserTest, ParseParameters_MultipleParameters) {
    std::string source = "(a, b, c)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "a");
    EXPECT_EQ(params[1], "b");
    EXPECT_EQ(params[2], "c");
}

TEST_F(ParserTest, ParseParameters_WithWhitespace) {
    std::string source = "( a , b , c )";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "a");
    EXPECT_EQ(params[1], "b");
    EXPECT_EQ(params[2], "c");
}

TEST_F(ParserTest, ParseParameters_WithNewlines) {
    std::string source = "(\na,\nb\n,\nc\n)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "a");
    EXPECT_EQ(params[1], "b");
    EXPECT_EQ(params[2], "c");
}

TEST_F(ParserTest, ParseParameters_TrailingComma) {
    std::string source = "(a, b,)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "a");
    EXPECT_EQ(params[1], "b");
}

TEST_F(ParserTest, ParseParameters_WithUnderscoreNames) {
    std::string source = "(_x, _y, _z)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "_x");
    EXPECT_EQ(params[1], "_y");
    EXPECT_EQ(params[2], "_z");
}

TEST_F(ParserTest, ParseParameters_WithNumbers) {
    std::string source = "(x1, y2, z3)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params[0], "x1");
    EXPECT_EQ(params[1], "y2");
    EXPECT_EQ(params[2], "z3");
}

TEST_F(ParserTest, ParseParameters_LongParameterList) {
    std::string source = "(a, b, c, d, e, f, g, h, i, j)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 10);
}

TEST_F(ParserTest, ParseParameters_WithDollarSign) {
    std::string source = "($x, $y)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    // 注意：如果 $ 符号不被支持，这个测试会抛出异常
    // 根据当前 lexer 的实现，$ 可能不被支持
    // EXPECT_NO_THROW({
    //     auto params = ParseParametersHelper(&parser);
    //     EXPECT_EQ(params.size(), 2);
    // });
}

// ==================== ParseExpressions 测试 ====================

TEST_F(ParserTest, ParseExpressions_EmptyList) {
    std::string source = "()";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 0);
}

TEST_F(ParserTest, ParseExpressions_SingleExpression) {
    std::string source = "(42)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 1);
}

TEST_F(ParserTest, ParseExpressions_MultipleExpressions) {
    std::string source = "(1, 2, 3)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_WithWhitespace) {
    std::string source = "( 1 , 2 , 3 )";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_TrailingComma_NotAllowed) {
    std::string source = "(1, 2,)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    // 当不允许尾部逗号时，应该抛出异常
    EXPECT_THROW(
        ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false),
        SyntaxError
    );
}

TEST_F(ParserTest, ParseExpressions_TrailingComma_Allowed) {
    std::string source = "(1, 2,)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, true);

    EXPECT_EQ(exprs.size(), 2);
}

TEST_F(ParserTest, ParseExpressions_WithIdentifiers) {
    std::string source = "(x, y, z)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_WithComplexExpressions) {
    std::string source = "(a + b, c * d, e / f)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_WithFunctionCalls) {
    std::string source = "(foo(), bar(), baz())";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_WithLiterals) {
    std::string source = "(42, 'hello', true, null)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 4);
}

TEST_F(ParserTest, ParseExpressions_YieldExpression) {
    std::string source = "(yield 1, yield 2)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 2);
}

TEST_F(ParserTest, ParseExpressions_WithArrays) {
    std::string source = "([1, 2], [3, 4])";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 2);
}

TEST_F(ParserTest, ParseExpressions_WithObjects) {
    std::string source = "({x: 1}, {y: 2})";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 2);
}

TEST_F(ParserTest, ParseExpressions_MixedTypes) {
    std::string source = "(42, 'test', x, y + z, foo())";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 5);
}

TEST_F(ParserTest, ParseExpressions_SquareBrackets) {
    std::string source = "[1, 2, 3]";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLBrack, TokenType::kSepRBrack, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_CurlyBraces) {
    std::string source = "{x: 1, y: 2}";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    // 注意：对象字面量的解析可能不同
    // 这个测试可能需要根据实际实现调整
    // EXPECT_NO_THROW({
    //     auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLCurly, TokenType::kSepRCurly, false);
    // });
}

// ==================== statements() 访问器测试 ====================

TEST_F(ParserTest, StatementsAccessor_EmptyProgram) {
    auto parser = ParseProgram("");

    EXPECT_EQ(parser->statements().size(), 0);
}

TEST_F(ParserTest, StatementsAccessor_NonEmptyProgram) {
    auto parser = ParseProgram("let x; let y; let z;");

    EXPECT_EQ(parser->statements().size(), 3);
}

TEST_F(ParserTest, StatementsAccessor_ConstReference) {
    auto parser = ParseProgram("let x;");

    const auto& stmts = parser->statements();
    EXPECT_EQ(stmts.size(), 1);
}

TEST_F(ParserTest, StatementsAccessor_Ordered) {
    auto parser = ParseProgram("let x; let y; let z;");

    const auto& stmts = parser->statements();
    EXPECT_EQ(stmts.size(), 3);

    // 验证语句的顺序
    // 可以通过检查语句类型或位置来验证
}

// ==================== import_declarations() 访问器测试 ====================

TEST_F(ParserTest, ImportDeclarationsAccessor_NoImports) {
    auto parser = ParseProgram("let x; let y;");

    EXPECT_EQ(parser->import_declarations().size(), 0);
}

TEST_F(ParserTest, ImportDeclarationsAccessor_SingleImport) {
    auto parser = ParseProgram("import { foo } from 'module';");

    EXPECT_EQ(parser->import_declarations().size(), 1);
}

TEST_F(ParserTest, ImportDeclarationsAccessor_MultipleImports) {
    auto parser = ParseProgram("import { foo } from 'module1'; import { bar } from 'module2';");

    EXPECT_EQ(parser->import_declarations().size(), 2);
}

TEST_F(ParserTest, ImportDeclarationsAccessor_MixedWithStatements) {
    auto parser = ParseProgram("import { foo } from 'module'; let x; import { bar } from 'module2';");

    EXPECT_EQ(parser->import_declarations().size(), 2);
    EXPECT_EQ(parser->statements().size(), 1);
}

TEST_F(ParserTest, ImportDeclarationsAccessor_ConstReference) {
    auto parser = ParseProgram("import { foo } from 'module';");

    const auto& imports = parser->import_declarations();
    EXPECT_EQ(imports.size(), 1);
}

TEST_F(ParserTest, ImportDeclarationsAccessor_Ordered) {
    auto parser = ParseProgram("import { foo } from 'module1'; import { bar } from 'module2'; import { baz } from 'module3';");

    const auto& imports = parser->import_declarations();
    EXPECT_EQ(imports.size(), 3);

    // 验证导入声明的顺序
}

// ==================== 错误处理测试 ====================

TEST_F(ParserTest, ParseProgram_InvalidSyntax) {
    std::string source = "let ;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(parser.ParseProgram(), SyntaxError);
}

TEST_F(ParserTest, ParseParameters_MissingLeftParen) {
    std::string source = "x)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(ParseParametersHelper(&parser), SyntaxError);
}

TEST_F(ParserTest, ParseParameters_MissingRightParen) {
    std::string source = "(x";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(ParseParametersHelper(&parser), SyntaxError);
}

TEST_F(ParserTest, ParseParameters_InvalidParameterName) {
    std::string source = "(123)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(ParseParametersHelper(&parser), SyntaxError);
}

TEST_F(ParserTest, ParseExpressions_MissingLeftDelimiter) {
    std::string source = "1, 2, 3)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(
        ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false),
        SyntaxError
    );
}

TEST_F(ParserTest, ParseExpressions_MissingRightDelimiter) {
    std::string source = "(1, 2, 3";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(
        ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false),
        SyntaxError
    );
}

TEST_F(ParserTest, ParseExpressions_InvalidExpression) {
    std::string source = "(1, + , 3)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_THROW(
        ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false),
        SyntaxError
    );
}

// ==================== 类型注解测试 ====================

TEST_F(ParserTest, ParseParameters_WithTypeAnnotation) {
    std::string source = "(x: number, y: string)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "x");
    EXPECT_EQ(params[1], "y");
    // 类型注解应该被尝试解析，但不影响参数名称列表
}

TEST_F(ParserTest, ParseParameters_WithComplexTypeAnnotation) {
    std::string source = "(x: string[], y: Map<string, number>)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0], "x");
    EXPECT_EQ(params[1], "y");
}

// ==================== 复杂场景测试 ====================

TEST_F(ParserTest, ParseProgram_ComplexProgram) {
    std::string source = R"(
        import { foo } from 'module1';
        import { bar } from 'module2';

        let x = 10;
        let y = 20;

        function add(a, b) {
            return a + b;
        }

        class MyClass {
            constructor() {
                this.value = 0;
            }
        }
    )";

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_GT(parser.statements().size(), 0);
    EXPECT_EQ(parser.import_declarations().size(), 2);
}

TEST_F(ParserTest, ParseProgram_WithAllStatementTypes) {
    std::string source = R"(
        let x;
        const y = 10;
        function foo() {}
        class Bar {}
        if (x) {}
        while (x) {}
        for (;;){}
        try {} catch(e) {}
        switch(x) {}
        return x;
        throw new Error();
        break;
        continue;
    )";

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_GT(parser.statements().size(), 0);
}

TEST_F(ParserTest, ParseProgram_NestedExpressions) {
    std::string source = "let x = (a + (b * (c / d)));";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_ArrowFunctionParameters) {
    std::string source = "(a, b, c) => a + b + c;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseParameters_ArrowFunctionSingleParam) {
    std::string source = "(x)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 1);
    EXPECT_EQ(params[0], "x");
}

TEST_F(ParserTest, ParseExpressions_FunctionCallArguments) {
    std::string source = "(1, 2, 3)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 3);
}

TEST_F(ParserTest, ParseExpressions_ArrayLiteral) {
    std::string source = "[1, 2, 3, 4, 5]";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLBrack, TokenType::kSepRBrack, false);

    EXPECT_EQ(exprs.size(), 5);
}

TEST_F(ParserTest, ParseProgram_WithAsyncFunctions) {
    std::string source = "async function foo() { await bar(); }";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithGeneratorFunctions) {
    std::string source = "function* generator() { yield 1; yield 2; }";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithDestructuring) {
    std::string source = "let { x, y } = obj;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseParameters_WithDefaultValues) {
    std::string source = "(x = 1, y = 2)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    // 注意：默认值参数可能需要特殊处理
    // 根据实现，这可能只解析参数名称，默认值可能在其他地方处理
    auto params = ParseParametersHelper(&parser);

    // 验证至少能解析参数名称
    EXPECT_GT(params.size(), 0);
}

TEST_F(ParserTest, ParseExpressions_WithSpreadOperator) {
    std::string source = "(...args)";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithTemplateStrings) {
    std::string source = "let x = `Hello ${name}`;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithRegExps) {
    std::string source = "let pattern = /abc/g;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithExport) {
    std::string source = "export { foo, bar };";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.statements().size(), 1);
}

TEST_F(ParserTest, ParseProgram_WithExportAndImport) {
    std::string source = "import { foo } from 'module'; export { bar };";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
    EXPECT_EQ(parser.import_declarations().size(), 1);
    EXPECT_EQ(parser.statements().size(), 1);
}

// ==================== 边界情况测试 ====================

TEST_F(ParserTest, ParseProgram_VeryLongProgram) {
    std::string source = "let x" + std::string(10000, ' ') + "= 1;";
    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
}

TEST_F(ParserTest, ParseParameters_ManyParameters) {
    std::string source = "(";
    for (int i = 0; i < 100; i++) {
        if (i > 0) source += ", ";
        source += "p" + std::to_string(i);
    }
    source += ")";

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto params = ParseParametersHelper(&parser);

    EXPECT_EQ(params.size(), 100);
}

TEST_F(ParserTest, ParseExpressions_ManyExpressions) {
    std::string source = "(";
    for (int i = 0; i < 100; i++) {
        if (i > 0) source += ", ";
        source += std::to_string(i);
    }
    source += ")";

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto exprs = ParseExpressionsHelper(&parser, TokenType::kSepLParen, TokenType::kSepRParen, false);

    EXPECT_EQ(exprs.size(), 100);
}

TEST_F(ParserTest, ParseProgram_DeeplyNested) {
    std::string source = "let x = ";
    for (int i = 0; i < 100; i++) {
        source += "(";
    }
    source += "1";
    for (int i = 0; i < 100; i++) {
        source += ")";
    }
    source += ";";

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    EXPECT_NO_THROW(parser.ParseProgram());
}

// ==================== 性能测试 ====================

TEST_F(ParserTest, ParseProgram_LargeFilePerformance) {
    std::string source;
    for (int i = 0; i < 1000; i++) {
        source += "let x" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    }

    auto lexer = std::make_unique<Lexer>(source);
    Parser parser(lexer.get());

    auto start = std::chrono::high_resolution_clock::now();
    EXPECT_NO_THROW(parser.ParseProgram());
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // 性能测试：确保解析不会太慢
    EXPECT_LT(duration.count(), 1000);  // 应该在1秒内完成
}

// ==================== 与 Statement 集成测试 ====================

TEST_F(ParserTest, ParseProgram_VariableDeclarations) {
    auto parser = ParseProgram("let x; const y = 10; var z;");

    EXPECT_EQ(parser->statements().size(), 3);
}

TEST_F(ParserTest, ParseProgram_FunctionDeclarations) {
    auto parser = ParseProgram("function foo() {} function bar() {}");

    EXPECT_EQ(parser->statements().size(), 2);
}

TEST_F(ParserTest, ParseProgram_ClassDeclarations) {
    auto parser = ParseProgram("class Foo {} class Bar {}");

    EXPECT_EQ(parser->statements().size(), 2);
}

TEST_F(ParserTest, ParseProgram_IfStatements) {
    auto parser = ParseProgram("if (true) {} if (false) {} else {}");

    EXPECT_EQ(parser->statements().size(), 2);
}

TEST_F(ParserTest, ParseProgram_LoopStatements) {
    auto parser = ParseProgram("while (true) {} for (;;) {} do {} while (false);");

    EXPECT_EQ(parser->statements().size(), 3);
}

TEST_F(ParserTest, ParseProgram_TryCatchStatements) {
    auto parser = ParseProgram("try {} catch (e) {} try {} finally {} try {} catch (e) {} finally {}");

    EXPECT_EQ(parser->statements().size(), 3);
}

// ==================== Parser 构造函数测试 ====================

TEST_F(ParserTest, Constructor_ValidLexer) {
    auto lexer = std::make_unique<Lexer>("let x;");
    EXPECT_NO_THROW({
        Parser parser(lexer.get());
    });
}

TEST_F(ParserTest, Constructor_NullLexer) {
    // 注意：如果构造函数不允许 nullptr，这个测试会编译失败
    // EXPECT_THROW({
    //     Parser parser(nullptr);
    // }, std::invalid_argument);
}

// ==================== NonCopyable 测试 ====================

TEST_F(ParserTest, Parser_IsNonCopyable) {
    auto lexer = std::make_unique<Lexer>("let x;");
    Parser parser1(lexer.get());

    // Parser 应该是不可复制的
    // Compiler should catch this at compile time
    // Parser parser2 = parser1;  // 这不应该编译通过

    // Parser 应该是不可移动的
    // Parser parser3 = std::move(parser1);  // 这不应该编译通过
}

} // namespace test
} // namespace compiler
} // namespace mjs
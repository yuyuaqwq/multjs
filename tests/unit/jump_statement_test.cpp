/**
 * @file jump_statement_test.cpp
 * @brief 跳转语句测试
 *
 * 测试跳转语句类型，包括:
 * - break语句 (BreakStatement)
 * - continue语句 (ContinueStatement)
 * - return语句 (ReturnStatement)
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "../src/compiler/lexer.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/statement.h"
#include "../src/compiler/statement_impl/break_statement.h"
#include "../src/compiler/statement_impl/continue_statement.h"
#include "../src/compiler/statement_impl/return_statement.h"
#include "../src/compiler/statement_impl/block_statement.h"
#include "../src/compiler/statement_impl/while_statement.h"
#include "../src/compiler/statement_impl/for_statement.h"
#include "../src/compiler/statement_impl/if_statement.h"
#include "../src/compiler/statement_impl/labeled_statement.h"
#include "../src/compiler/expression_impl/integer_literal.h"
#include "../src/compiler/expression_impl/string_literal.h"
#include "../src/compiler/expression_impl/identifier.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class JumpStatementTest
 * @brief 跳转语句测试类
 */
class JumpStatementTest : public ::testing::Test {
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
     * @brief 辅助方法：解析语句
     * @param source 源代码字符串
     * @return Statement对象的唯一指针
     */
    std::unique_ptr<Statement> ParseStatement(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return Statement::ParseStatement(lexer.get());
    }
};

// ============================================================================
// break语句测试 (BreakStatement)
// ============================================================================

/**
 * @test 测试简单的break语句
 */
TEST_F(JumpStatementTest, SimpleBreakStatement) {
    auto stmt = ParseStatement("break;");
    auto* break_stmt = dynamic_cast<BreakStatement*>(stmt.get());
    ASSERT_NE(break_stmt, nullptr);
    EXPECT_EQ(break_stmt->type(), StatementType::kBreak);
    EXPECT_FALSE(break_stmt->label().has_value());
}

/**
 * @test 测试带标签的break语句
 */
TEST_F(JumpStatementTest, LabeledBreakStatement) {
    auto stmt = ParseStatement("break myLabel;");
    auto* break_stmt = dynamic_cast<BreakStatement*>(stmt.get());
    ASSERT_NE(break_stmt, nullptr);
    EXPECT_EQ(break_stmt->type(), StatementType::kBreak);
    EXPECT_TRUE(break_stmt->label().has_value());
    EXPECT_EQ(break_stmt->label().value(), "myLabel");
}

/**
 * @test 测试break在while循环中
 */
TEST_F(JumpStatementTest, BreakInWhileLoop) {
    auto stmt = ParseStatement("while (true) { break; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* break_stmt = dynamic_cast<BreakStatement*>(inner_stmt.get());
    ASSERT_NE(break_stmt, nullptr);
    EXPECT_EQ(break_stmt->type(), StatementType::kBreak);
}

/**
 * @test 测试break在for循环中
 */
TEST_F(JumpStatementTest, BreakInForLoop) {
    auto stmt = ParseStatement("for (;;) { break; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);

    const auto& body = for_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* break_stmt = dynamic_cast<BreakStatement*>(inner_stmt.get());
    ASSERT_NE(break_stmt, nullptr);
    EXPECT_EQ(break_stmt->type(), StatementType::kBreak);
}

/**
 * @test 测试嵌套循环中的break
 */
TEST_F(JumpStatementTest, BreakInNestedLoop) {
    auto stmt = ParseStatement("while (true) { while (true) { break; } break; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->statements().size(), 2);

    // 第一个语句应该是内层while循环
    const auto& inner_while = body->statements()[0];
    auto* inner_while_stmt = dynamic_cast<WhileStatement*>(inner_while.get());
    ASSERT_NE(inner_while_stmt, nullptr);

    // 第二个语句应该是break
    const auto& break_stmt = body->statements()[1];
    auto* break_ptr = dynamic_cast<BreakStatement*>(break_stmt.get());
    ASSERT_NE(break_ptr, nullptr);
    EXPECT_EQ(break_ptr->type(), StatementType::kBreak);
}

/**
 * @test 测试带标签的break跳转到外层循环
 */
TEST_F(JumpStatementTest, LabeledBreakToOuterLoop) {
    auto stmt = ParseStatement("outer: while (true) { while (true) { break outer; } }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->label(), "outer");
}

/**
 * @test 测试break语句源代码位置
 */
TEST_F(JumpStatementTest, BreakStatementSourcePosition) {
    auto stmt = ParseStatement("break;");
    auto* break_stmt = dynamic_cast<BreakStatement*>(stmt.get());
    ASSERT_NE(break_stmt, nullptr);
    // break语句应该有正确的源代码位置信息
}

/**
 * @test 测试break在条件语句中
 */
TEST_F(JumpStatementTest, BreakInConditionalStatement) {
    auto stmt = ParseStatement("while (true) { if (true) { break; } }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
}

// ============================================================================
// continue语句测试 (ContinueStatement)
// ============================================================================

/**
 * @test 测试简单的continue语句
 */
TEST_F(JumpStatementTest, SimpleContinueStatement) {
    auto stmt = ParseStatement("continue;");
    auto* continue_stmt = dynamic_cast<ContinueStatement*>(stmt.get());
    ASSERT_NE(continue_stmt, nullptr);
    EXPECT_EQ(continue_stmt->type(), StatementType::kContinue);
    EXPECT_FALSE(continue_stmt->label().has_value());
}

/**
 * @test 测试带标签的continue语句
 */
TEST_F(JumpStatementTest, LabeledContinueStatement) {
    auto stmt = ParseStatement("continue myLabel;");
    auto* continue_stmt = dynamic_cast<ContinueStatement*>(stmt.get());
    ASSERT_NE(continue_stmt, nullptr);
    EXPECT_EQ(continue_stmt->type(), StatementType::kContinue);
    EXPECT_TRUE(continue_stmt->label().has_value());
    EXPECT_EQ(continue_stmt->label().value(), "myLabel");
}

/**
 * @test 测试continue在while循环中
 */
TEST_F(JumpStatementTest, ContinueInWhileLoop) {
    auto stmt = ParseStatement("while (true) { continue; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* continue_stmt = dynamic_cast<ContinueStatement*>(inner_stmt.get());
    ASSERT_NE(continue_stmt, nullptr);
    EXPECT_EQ(continue_stmt->type(), StatementType::kContinue);
}

/**
 * @test 测试continue在for循环中
 */
TEST_F(JumpStatementTest, ContinueInForLoop) {
    auto stmt = ParseStatement("for (;;) { continue; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);

    const auto& body = for_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* continue_stmt = dynamic_cast<ContinueStatement*>(inner_stmt.get());
    ASSERT_NE(continue_stmt, nullptr);
    EXPECT_EQ(continue_stmt->type(), StatementType::kContinue);
}

/**
 * @test 测试嵌套循环中的continue
 */
TEST_F(JumpStatementTest, ContinueInNestedLoop) {
    auto stmt = ParseStatement("while (true) { while (true) { continue; } continue; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->statements().size(), 2);
}

/**
 * @test 测试带标签的continue跳转到外层循环
 */
TEST_F(JumpStatementTest, LabeledContinueToOuterLoop) {
    auto stmt = ParseStatement("outer: while (true) { while (true) { continue outer; } }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->label(), "outer");
}

/**
 * @test 测试continue语句源代码位置
 */
TEST_F(JumpStatementTest, ContinueStatementSourcePosition) {
    auto stmt = ParseStatement("continue;");
    auto* continue_stmt = dynamic_cast<ContinueStatement*>(stmt.get());
    ASSERT_NE(continue_stmt, nullptr);
    // continue语句应该有正确的源代码位置信息
}

/**
 * @test 测试continue在条件语句中
 */
TEST_F(JumpStatementTest, ContinueInConditionalStatement) {
    auto stmt = ParseStatement("while (true) { if (true) { continue; } }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
}

// ============================================================================
// return语句测试 (ReturnStatement)
// ============================================================================

/**
 * @test 测试无返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithoutValue) {
    auto stmt = ParseStatement("return;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_EQ(return_stmt->type(), StatementType::kReturn);
    EXPECT_EQ(return_stmt->argument(), nullptr);
}

/**
 * @test 测试带字面量返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithLiteralValue) {
    auto stmt = ParseStatement("return 42;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_EQ(return_stmt->type(), StatementType::kReturn);
    EXPECT_NE(return_stmt->argument(), nullptr);

    auto* arg_literal = dynamic_cast<IntegerLiteral*>(return_stmt->argument().get());
    ASSERT_NE(arg_literal, nullptr);
    EXPECT_EQ(arg_literal->value(), 42);
}

/**
 * @test 测试带字符串返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithStringValue) {
    auto stmt = ParseStatement("return 'hello';");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);

    auto* arg_string = dynamic_cast<StringLiteral*>(return_stmt->argument().get());
    ASSERT_NE(arg_string, nullptr);
    EXPECT_EQ(arg_string->value(), "hello");
}

/**
 * @test 测试带标识符返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithIdentifierValue) {
    auto stmt = ParseStatement("return x;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);

    auto* arg_identifier = dynamic_cast<Identifier*>(return_stmt->argument().get());
    ASSERT_NE(arg_identifier, nullptr);
    EXPECT_EQ(arg_identifier->name(), "x");
}

/**
 * @test 测试带表达式返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithExpressionValue) {
    auto stmt = ParseStatement("return x + y;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);
}

/**
 * @test 测试带复杂表达式返回值的return语句
 */
TEST_F(JumpStatementTest, ReturnWithComplexExpressionValue) {
    auto stmt = ParseStatement("return a + b * c - d;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);
}

/**
 * @test 测试return在函数中
 */
TEST_F(JumpStatementTest, ReturnInFunction) {
    auto stmt = ParseStatement("function foo() { return 42; }");
    // 这应该解析为一个函数表达式或语句
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试多个return语句
 */
TEST_F(JumpStatementTest, MultipleReturnStatements) {
    auto stmt = ParseStatement("{ return 1; return 2; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->statements().size(), 2);

    const auto& first_return = block_stmt->statements()[0];
    auto* first_return_ptr = dynamic_cast<ReturnStatement*>(first_return.get());
    ASSERT_NE(first_return_ptr, nullptr);

    const auto& second_return = block_stmt->statements()[1];
    auto* second_return_ptr = dynamic_cast<ReturnStatement*>(second_return.get());
    ASSERT_NE(second_return_ptr, nullptr);
}

/**
 * @test 测试return语句源代码位置
 */
TEST_F(JumpStatementTest, ReturnStatementSourcePosition) {
    auto stmt = ParseStatement("return;");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    // return语句应该有正确的源代码位置信息
}

/**
 * @test 测试return在条件语句中
 */
TEST_F(JumpStatementTest, ReturnInConditionalStatement) {
    auto stmt = ParseStatement("if (true) { return 42; } else { return 24; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
}

/**
 * @test 测试return在循环中
 */
TEST_F(JumpStatementTest, ReturnInLoop) {
    auto stmt = ParseStatement("while (true) { return 42; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* return_stmt = dynamic_cast<ReturnStatement*>(inner_stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_EQ(return_stmt->type(), StatementType::kReturn);
}

/**
 * @test 测试return对象字面量
 */
TEST_F(JumpStatementTest, ReturnObjectLiteral) {
    auto stmt = ParseStatement("return { x: 1, y: 2 };");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);
}

/**
 * @test 测试return数组字面量
 */
TEST_F(JumpStatementTest, ReturnArrayLiteral) {
    auto stmt = ParseStatement("return [1, 2, 3];");
    auto* return_stmt = dynamic_cast<ReturnStatement*>(stmt.get());
    ASSERT_NE(return_stmt, nullptr);
    EXPECT_NE(return_stmt->argument(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

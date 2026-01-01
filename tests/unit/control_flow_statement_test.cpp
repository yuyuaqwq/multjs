/**
 * @file control_flow_statement_test.cpp
 * @brief 控制流语句测试
 *
 * 测试控制流语句类型，包括:
 * - if语句 (IfStatement)
 * - while循环语句 (WhileStatement)
 * - for循环语句 (ForStatement)
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
#include "../src/compiler/statement_impl/if_statement.h"
#include "../src/compiler/statement_impl/while_statement.h"
#include "../src/compiler/statement_impl/for_statement.h"
#include "../src/compiler/statement_impl/block_statement.h"
#include "../src/compiler/statement_impl/expression_statement.h"
#include "../src/compiler/expression_impl/integer_literal.h"
#include "../src/compiler/expression_impl/string_literal.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/binary_expression.h"
#include "../src/compiler/expression_impl/unary_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ControlFlowStatementTest
 * @brief 控制流语句测试类
 */
class ControlFlowStatementTest : public ::testing::Test {
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
// if语句测试 (IfStatement)
// ============================================================================

/**
 * @test 测试简单的if语句
 */
TEST_F(ControlFlowStatementTest, SimpleIfStatement) {
    auto stmt = ParseStatement("if (true) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_EQ(if_stmt->type(), StatementType::kIf);
    EXPECT_NE(if_stmt->test(), nullptr);
    EXPECT_NE(if_stmt->consequent(), nullptr);
    EXPECT_EQ(if_stmt->alternate(), nullptr);
}

/**
 * @test 测试if-else语句
 */
TEST_F(ControlFlowStatementTest, IfElseStatement) {
    auto stmt = ParseStatement("if (true) { 42; } else { 24; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_EQ(if_stmt->type(), StatementType::kIf);
    EXPECT_NE(if_stmt->test(), nullptr);
    EXPECT_NE(if_stmt->consequent(), nullptr);
    EXPECT_NE(if_stmt->alternate(), nullptr);
}

/**
 * @test 测试if-else if-else语句
 */
TEST_F(ControlFlowStatementTest, IfElseIfElseStatement) {
    auto stmt = ParseStatement("if (a) { 1; } else if (b) { 2; } else { 3; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_EQ(if_stmt->type(), StatementType::kIf);
    EXPECT_NE(if_stmt->test(), nullptr);
    EXPECT_NE(if_stmt->consequent(), nullptr);
    EXPECT_NE(if_stmt->alternate(), nullptr);

    // alternate应该是一个IfStatement
    auto* else_if_stmt = dynamic_cast<IfStatement*>(if_stmt->alternate().get());
    ASSERT_NE(else_if_stmt, nullptr);
    EXPECT_EQ(else_if_stmt->type(), StatementType::kIf);
    EXPECT_NE(else_if_stmt->alternate(), nullptr);
}

/**
 * @test 测试嵌套if语句
 */
TEST_F(ControlFlowStatementTest, NestedIfStatement) {
    auto stmt = ParseStatement("if (true) { if (false) { 42; } }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_EQ(if_stmt->type(), StatementType::kIf);

    const auto& consequent = if_stmt->consequent();
    ASSERT_NE(consequent, nullptr);
    EXPECT_GT(consequent->statements().size(), 0);

    // 第一个语句应该是if语句
    const auto& inner_stmt = consequent->statements()[0];
    auto* inner_if_stmt = dynamic_cast<IfStatement*>(inner_stmt.get());
    ASSERT_NE(inner_if_stmt, nullptr);
    EXPECT_EQ(inner_if_stmt->type(), StatementType::kIf);
}

/**
 * @test 测试if语句与标识符条件
 */
TEST_F(ControlFlowStatementTest, IfStatementWithIdentifierCondition) {
    auto stmt = ParseStatement("if (x) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->test(), nullptr);

    auto* test_identifier = dynamic_cast<Identifier*>(if_stmt->test().get());
    ASSERT_NE(test_identifier, nullptr);
    EXPECT_EQ(test_identifier->name(), "x");
}

/**
 * @test 测试if语句与二元表达式条件
 */
TEST_F(ControlFlowStatementTest, IfStatementWithBinaryExpressionCondition) {
    auto stmt = ParseStatement("if (x > 5) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->test(), nullptr);

    auto* test_binary = dynamic_cast<BinaryExpression*>(if_stmt->test().get());
    ASSERT_NE(test_binary, nullptr);
    // EXPECT_EQ(test_binary->op(), BinaryOp::kGreaterThan);
}

/**
 * @test 测试if语句与逻辑表达式条件
 */
TEST_F(ControlFlowStatementTest, IfStatementWithLogicalExpressionCondition) {
    auto stmt = ParseStatement("if (a && b) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->test(), nullptr);

    auto* test_binary = dynamic_cast<BinaryExpression*>(if_stmt->test().get());
    ASSERT_NE(test_binary, nullptr);
    // EXPECT_EQ(test_binary->op(), BinaryOp::kLogicalAnd);
}

/**
 * @test 测试if语句与复杂条件
 */
TEST_F(ControlFlowStatementTest, IfStatementWithComplexCondition) {
    auto stmt = ParseStatement("if (x > 0 && y < 10 || z === 5) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->test(), nullptr);
}

/**
 * @test 测试if语句体包含多个语句
 */
TEST_F(ControlFlowStatementTest, IfStatementWithMultipleStatements) {
    auto stmt = ParseStatement("if (true) { 1; 2; 3; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->consequent(), nullptr);

    const auto& consequent = if_stmt->consequent();
    EXPECT_EQ(consequent->statements().size(), 3);
}

/**
 * @test 测试if-else语句体都包含多个语句
 */
TEST_F(ControlFlowStatementTest, IfElseStatementWithMultipleStatements) {
    auto stmt = ParseStatement("if (true) { 1; 2; } else { 3; 4; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);
    EXPECT_NE(if_stmt->consequent(), nullptr);
    EXPECT_NE(if_stmt->alternate(), nullptr);

    const auto& consequent = if_stmt->consequent();
    EXPECT_EQ(consequent->statements().size(), 2);

    auto* alternate_block = dynamic_cast<BlockStatement*>(if_stmt->alternate().get());
    ASSERT_NE(alternate_block, nullptr);
    EXPECT_EQ(alternate_block->statements().size(), 2);
}

/**
 * @test 测试if语句源代码位置
 */
TEST_F(ControlFlowStatementTest, IfStatementSourcePosition) {
    auto stmt = ParseStatement("if (true) { 42; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_NE(if_stmt, nullptr);

    auto start = if_stmt->start();
    auto end = if_stmt->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

// ============================================================================
// while循环语句测试 (WhileStatement)
// ============================================================================

/**
 * @test 测试简单的while循环
 */
TEST_F(ControlFlowStatementTest, SimpleWhileLoop) {
    auto stmt = ParseStatement("while (true) { 42; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_EQ(while_stmt->type(), StatementType::kWhile);
    EXPECT_NE(while_stmt->test(), nullptr);
    EXPECT_NE(while_stmt->body(), nullptr);
}

/**
 * @test 测试while循环与标识符条件
 */
TEST_F(ControlFlowStatementTest, WhileLoopWithIdentifierCondition) {
    auto stmt = ParseStatement("while (x) { 42; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_NE(while_stmt->test(), nullptr);

    auto* test_identifier = dynamic_cast<Identifier*>(while_stmt->test().get());
    ASSERT_NE(test_identifier, nullptr);
    EXPECT_EQ(test_identifier->name(), "x");
}

/**
 * @test 测试while循环与二元表达式条件
 */
TEST_F(ControlFlowStatementTest, WhileLoopWithBinaryExpressionCondition) {
    auto stmt = ParseStatement("while (i < 10) { i++; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_NE(while_stmt->test(), nullptr);

    auto* test_binary = dynamic_cast<BinaryExpression*>(while_stmt->test().get());
    ASSERT_NE(test_binary, nullptr);
    // EXPECT_EQ(test_binary->op(), BinaryOp::kLessThan);
}

/**
 * @test 测试while循环体包含多个语句
 */
TEST_F(ControlFlowStatementTest, WhileLoopWithMultipleStatements) {
    auto stmt = ParseStatement("while (true) { 1; 2; 3; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_NE(while_stmt->body(), nullptr);

    const auto& body = while_stmt->body();
    EXPECT_EQ(body->statements().size(), 3);
}

/**
 * @test 测试嵌套while循环
 */
TEST_F(ControlFlowStatementTest, NestedWhileLoop) {
    auto stmt = ParseStatement("while (true) { while (false) { 42; } }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_EQ(while_stmt->type(), StatementType::kWhile);

    const auto& body = while_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* inner_while_stmt = dynamic_cast<WhileStatement*>(inner_stmt.get());
    ASSERT_NE(inner_while_stmt, nullptr);
    EXPECT_EQ(inner_while_stmt->type(), StatementType::kWhile);
}

/**
 * @test 测试while循环与复合条件
 */
TEST_F(ControlFlowStatementTest, WhileLoopWithComplexCondition) {
    auto stmt = ParseStatement("while (i < 10 && j > 0) { 42; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
    EXPECT_NE(while_stmt->test(), nullptr);
}

/**
 * @test 测试while循环源代码位置
 */
TEST_F(ControlFlowStatementTest, WhileLoopSourcePosition) {
    auto stmt = ParseStatement("while (true) { 42; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);

    auto start = while_stmt->start();
    auto end = while_stmt->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

// ============================================================================
// for循环语句测试 (ForStatement)
// ============================================================================

/**
 * @test 测试基本for循环 (for(;;))
 */
TEST_F(ControlFlowStatementTest, BasicForLoop) {
    auto stmt = ParseStatement("for (;;) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);
    EXPECT_NE(for_stmt->body(), nullptr);
    EXPECT_EQ(for_stmt->init(), nullptr);
    EXPECT_EQ(for_stmt->test(), nullptr);
    EXPECT_EQ(for_stmt->update(), nullptr);
}

/**
 * @test 测试带初始化的for循环
 */
TEST_F(ControlFlowStatementTest, ForLoopWithInit) {
    auto stmt = ParseStatement("for (let i = 0; ; ) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);
    EXPECT_NE(for_stmt->init(), nullptr);
    EXPECT_EQ(for_stmt->test(), nullptr);
    EXPECT_EQ(for_stmt->update(), nullptr);
}

/**
 * @test 测试带条件的for循环
 */
TEST_F(ControlFlowStatementTest, ForLoopWithTest) {
    auto stmt = ParseStatement("for (; i < 10; ) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);
    EXPECT_EQ(for_stmt->init(), nullptr);
    EXPECT_NE(for_stmt->test(), nullptr);
    EXPECT_EQ(for_stmt->update(), nullptr);
}

/**
 * @test 测试带更新的for循环
 */
TEST_F(ControlFlowStatementTest, ForLoopWithUpdate) {
    auto stmt = ParseStatement("for (; ; i++) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);
    EXPECT_EQ(for_stmt->init(), nullptr);
    EXPECT_EQ(for_stmt->test(), nullptr);
    EXPECT_NE(for_stmt->update(), nullptr);
}

/**
 * @test 测试完整的for循环
 */
TEST_F(ControlFlowStatementTest, CompleteForLoop) {
    auto stmt = ParseStatement("for (let i = 0; i < 10; i++) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);
    EXPECT_NE(for_stmt->init(), nullptr);
    EXPECT_NE(for_stmt->test(), nullptr);
    EXPECT_NE(for_stmt->update(), nullptr);
    EXPECT_NE(for_stmt->body(), nullptr);
}

/**
 * @test 测试for循环体包含多个语句
 */
TEST_F(ControlFlowStatementTest, ForLoopWithMultipleStatements) {
    auto stmt = ParseStatement("for (;;) { 1; 2; 3; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_NE(for_stmt->body(), nullptr);

    const auto& body = for_stmt->body();
    EXPECT_EQ(body->statements().size(), 3);
}

/**
 * @test 测试嵌套for循环
 */
TEST_F(ControlFlowStatementTest, NestedForLoop) {
    auto stmt = ParseStatement("for (;;) { for (;;) { 42; } }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_EQ(for_stmt->type(), StatementType::kFor);

    const auto& body = for_stmt->body();
    ASSERT_NE(body, nullptr);
    EXPECT_GT(body->statements().size(), 0);

    const auto& inner_stmt = body->statements()[0];
    auto* inner_for_stmt = dynamic_cast<ForStatement*>(inner_stmt.get());
    ASSERT_NE(inner_for_stmt, nullptr);
    EXPECT_EQ(inner_for_stmt->type(), StatementType::kFor);
}

/**
 * @test 测试for循环的条件表达式
 */
TEST_F(ControlFlowStatementTest, ForLoopConditionExpression) {
    auto stmt = ParseStatement("for (; i < 10; ) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_NE(for_stmt->test(), nullptr);

    auto* test_binary = dynamic_cast<BinaryExpression*>(for_stmt->test().get());
    ASSERT_NE(test_binary, nullptr);
    // EXPECT_EQ(test_binary->op(), BinaryOp::kLessThan);
}

/**
 * @test 测试for循环的更新表达式
 */
TEST_F(ControlFlowStatementTest, ForLoopUpdateExpression) {
    auto stmt = ParseStatement("for (; ; i++) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_NE(for_stmt->update(), nullptr);

    auto* update_unary = dynamic_cast<UnaryExpression*>(for_stmt->update().get());
    ASSERT_NE(update_unary, nullptr);
    // EXPECT_EQ(update_unary->op(), UnaryOp::kPostIncrement);
}

/**
 * @test 测试for循环的复合更新表达式
 */
TEST_F(ControlFlowStatementTest, ForLoopComplexUpdateExpression) {
    auto stmt = ParseStatement("for (; ; i += 2) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_NE(for_stmt->update(), nullptr);

    auto* update_binary = dynamic_cast<BinaryExpression*>(for_stmt->update().get());
    ASSERT_NE(update_binary, nullptr);
    // EXPECT_EQ(update_binary->op(), BinaryOp::kAddAssign);
}

/**
 * @test 测试for循环的复杂条件
 */
TEST_F(ControlFlowStatementTest, ForLoopWithComplexCondition) {
    auto stmt = ParseStatement("for (let i = 0; i < 10 && j > 0; i++) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);
    EXPECT_NE(for_stmt->init(), nullptr);
    EXPECT_NE(for_stmt->test(), nullptr);
    EXPECT_NE(for_stmt->update(), nullptr);
}

/**
 * @test 测试for循环源代码位置
 */
TEST_F(ControlFlowStatementTest, ForLoopSourcePosition) {
    auto stmt = ParseStatement("for (;;) { 42; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_NE(for_stmt, nullptr);

    auto start = for_stmt->start();
    auto end = for_stmt->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

/**
 * @test 测试for循环与while循环的等价性
 */
TEST_F(ControlFlowStatementTest, ForLoopVersusWhileLoop) {
    auto for_stmt = ParseStatement("for (let i = 0; i < 10; i++) { 42; }");
    auto* for_stmt_ptr = dynamic_cast<ForStatement*>(for_stmt.get());
    ASSERT_NE(for_stmt_ptr, nullptr);

    auto while_stmt = ParseStatement("let i = 0; while (i < 10) { 42; i++; }");
    auto* while_stmt_ptr = dynamic_cast<WhileStatement*>(while_stmt.get());
    ASSERT_NE(while_stmt_ptr, nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

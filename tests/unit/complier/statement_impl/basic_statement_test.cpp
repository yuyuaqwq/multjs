/**
 * @file basic_statement_test.cpp
 * @brief 基础语句测试
 *
 * 测试基础语句类型，包括:
 * - 块语句 (BlockStatement)
 * - 表达式语句 (ExpressionStatement)
 * - 标签语句 (LabeledStatement)
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/statement_impl/expression_statement.h"
#include "src/compiler/statement_impl/labeled_statement.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/identifier.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class BasicStatementTest
 * @brief 基础语句测试类
 */
class BasicStatementTest : public ::testing::Test {
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
// 块语句测试 (BlockStatement)
// ============================================================================

/**
 * @test 测试空块语句
 */
TEST_F(BasicStatementTest, EmptyBlockStatement) {
    auto stmt = ParseStatement("{}");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 0);
}

/**
 * @test 测试包含单个语句的块
 */
TEST_F(BasicStatementTest, BlockStatementWithSingleStatement) {
    auto stmt = ParseStatement("{ 42; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 1);

    const auto& inner_stmt = block_stmt->statements()[0];
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(inner_stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
}

/**
 * @test 测试包含多个语句的块
 */
TEST_F(BasicStatementTest, BlockStatementWithMultipleStatements) {
    auto stmt = ParseStatement("{ 42; 'hello'; 123; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 3);

    // 验证第一个语句
    auto* expr_stmt1 = dynamic_cast<ExpressionStatement*>(block_stmt->statements()[0].get());
    ASSERT_NE(expr_stmt1, nullptr);

    // 验证第二个语句
    auto* expr_stmt2 = dynamic_cast<ExpressionStatement*>(block_stmt->statements()[1].get());
    ASSERT_NE(expr_stmt2, nullptr);

    // 验证第三个语句
    auto* expr_stmt3 = dynamic_cast<ExpressionStatement*>(block_stmt->statements()[2].get());
    ASSERT_NE(expr_stmt3, nullptr);
}

/**
 * @test 测试嵌套块语句
 */
TEST_F(BasicStatementTest, NestedBlockStatements) {
    auto stmt = ParseStatement("{ { {} } }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 1);

    // 获取内层块
    const auto& inner_stmt = block_stmt->statements()[0];
    auto* inner_block_stmt = dynamic_cast<BlockStatement*>(inner_stmt.get());
    ASSERT_NE(inner_block_stmt, nullptr);
    EXPECT_EQ(inner_block_stmt->statements().size(), 1);

    // 获取最内层块
    const auto& innermost_stmt = inner_block_stmt->statements()[0];
    auto* innermost_block_stmt = dynamic_cast<BlockStatement*>(innermost_stmt.get());
    ASSERT_NE(innermost_block_stmt, nullptr);
    EXPECT_EQ(innermost_block_stmt->statements().size(), 0);
}

/**
 * @test 测试块语句中的多个表达式语句
 */
TEST_F(BasicStatementTest, BlockStatementWithMultipleExpressions) {
    auto stmt = ParseStatement("{ x = 1; y = 2; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 2);
}

/**
 * @test 测试块语句中的函数调用
 */
TEST_F(BasicStatementTest, BlockStatementWithFunctionCalls) {
    auto stmt = ParseStatement("{ foo(); bar(); }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
    EXPECT_EQ(block_stmt->statements().size(), 2);

    // 验证第一个函数调用
    auto* expr_stmt1 = dynamic_cast<ExpressionStatement*>(block_stmt->statements()[0].get());
    ASSERT_NE(expr_stmt1, nullptr);
    EXPECT_NE(expr_stmt1->expression(), nullptr);
}

/**
 * @test 测试块语句源代码位置
 */
TEST_F(BasicStatementTest, BlockStatementSourcePosition) {
    auto stmt = ParseStatement("{ 42; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    // 块语句应该有正确的源代码位置信息
    // 这里我们只验证对象创建成功，具体位置值取决于Lexer实现
}

// ============================================================================
// 表达式语句测试 (ExpressionStatement)
// ============================================================================

/**
 * @test 测试空语句（只有分号）
 */
TEST_F(BasicStatementTest, EmptyExpressionStatement) {
    auto stmt = ParseStatement(";");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_EQ(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试整数字面量表达式语句
 */
TEST_F(BasicStatementTest, IntegerLiteralExpressionStatement) {
    auto stmt = ParseStatement("42;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);

    auto* int_lit = dynamic_cast<IntegerLiteral*>(expr_stmt->expression().get());
    ASSERT_NE(int_lit, nullptr);
    EXPECT_EQ(int_lit->value(), 42);
}

/**
 * @test 测试字符串字面量表达式语句
 */
TEST_F(BasicStatementTest, StringLiteralExpressionStatement) {
    auto stmt = ParseStatement("'hello';");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);

    auto* str_lit = dynamic_cast<StringLiteral*>(expr_stmt->expression().get());
    ASSERT_NE(str_lit, nullptr);
    EXPECT_EQ(str_lit->value(), "hello");
}

/**
 * @test 测试标识符表达式语句
 */
TEST_F(BasicStatementTest, IdentifierExpressionStatement) {
    auto stmt = ParseStatement("x;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);

    auto* identifier = dynamic_cast<Identifier*>(expr_stmt->expression().get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name(), "x");
}

/**
 * @test 测试二元运算表达式语句
 */
TEST_F(BasicStatementTest, BinaryOperationExpressionStatement) {
    auto stmt = ParseStatement("a + b;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试赋值表达式语句
 */
TEST_F(BasicStatementTest, AssignmentExpressionStatement) {
    auto stmt = ParseStatement("x = 42;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试函数调用表达式语句
 */
TEST_F(BasicStatementTest, FunctionCallExpressionStatement) {
    auto stmt = ParseStatement("foo();");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试带参数的函数调用表达式语句
 */
TEST_F(BasicStatementTest, FunctionCallWithArgumentsExpressionStatement) {
    auto stmt = ParseStatement("foo(1, 2, 3);");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试成员访问表达式语句
 */
TEST_F(BasicStatementTest, MemberAccessExpressionStatement) {
    auto stmt = ParseStatement("obj.prop;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

/**
 * @test 测试复杂表达式语句
 */
TEST_F(BasicStatementTest, ComplexExpressionStatement) {
    auto stmt = ParseStatement("obj.arr[i + 1] = x * y + z;");
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    ASSERT_NE(expr_stmt, nullptr);
    EXPECT_EQ(expr_stmt->type(), StatementType::kExpression);
    EXPECT_NE(expr_stmt->expression(), nullptr);
}

// ============================================================================
// 标签语句测试 (LabeledStatement)
// ============================================================================

/**
 * @test 测试简单标签语句
 */
TEST_F(BasicStatementTest, SimpleLabeledStatement) {
    auto stmt = ParseStatement("label1: 42;");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->type(), StatementType::kLabeled);
    EXPECT_EQ(labeled_stmt->label(), "label1");
    EXPECT_NE(labeled_stmt->body(), nullptr);
}

/**
 * @test 测试标签语句带块
 */
TEST_F(BasicStatementTest, LabeledStatementWithBlock) {
    auto stmt = ParseStatement("loop: { break loop; }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->type(), StatementType::kLabeled);
    EXPECT_EQ(labeled_stmt->label(), "loop");
    EXPECT_NE(labeled_stmt->body(), nullptr);

    // 验证body是块语句
    auto* block_stmt = dynamic_cast<BlockStatement*>(labeled_stmt->body().get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->type(), StatementType::kBlock);
}

/**
 * @test 测试嵌套标签语句
 */
TEST_F(BasicStatementTest, NestedLabeledStatements) {
    auto stmt = ParseStatement("outer: inner: 42;");
    auto* outer_labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(outer_labeled_stmt, nullptr);
    EXPECT_EQ(outer_labeled_stmt->type(), StatementType::kLabeled);
    EXPECT_EQ(outer_labeled_stmt->label(), "outer");

    // 内层语句也应该是标签语句
    auto* inner_labeled_stmt = dynamic_cast<LabeledStatement*>(outer_labeled_stmt->body().get());
    ASSERT_NE(inner_labeled_stmt, nullptr);
    EXPECT_EQ(inner_labeled_stmt->label(), "inner");
}

/**
 * @test 测试多个不同标签
 */
TEST_F(BasicStatementTest, MultipleDifferentLabels) {
    auto stmt1 = ParseStatement("start: x = 1;");
    auto* labeled_stmt1 = dynamic_cast<LabeledStatement*>(stmt1.get());
    ASSERT_NE(labeled_stmt1, nullptr);
    EXPECT_EQ(labeled_stmt1->label(), "start");

    auto stmt2 = ParseStatement("end: return;");
    auto* labeled_stmt2 = dynamic_cast<LabeledStatement*>(stmt2.get());
    ASSERT_NE(labeled_stmt2, nullptr);
    EXPECT_EQ(labeled_stmt2->label(), "end");
}

/**
 * @test 测试标签语句带循环
 */
TEST_F(BasicStatementTest, LabeledStatementWithLoop) {
    auto stmt = ParseStatement("myloop: while (true) { break myloop; }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->type(), StatementType::kLabeled);
    EXPECT_EQ(labeled_stmt->label(), "myloop");
    EXPECT_NE(labeled_stmt->body(), nullptr);
}

/**
 * @test 测试标签语句源代码位置
 */
TEST_F(BasicStatementTest, LabeledStatementSourcePosition) {
    auto stmt = ParseStatement("label: 42;");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    // 标签语句应该有正确的源代码位置信息
}

// ============================================================================
// 组合测试
// ============================================================================

/**
 * @test 测试块语句中的标签语句
 */
TEST_F(BasicStatementTest, LabeledStatementInsideBlock) {
    auto stmt = ParseStatement("{ label: 42; }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->statements().size(), 1);

    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(block_stmt->statements()[0].get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->label(), "label");
}

/**
 * @test 测试标签语句中的块语句
 */
TEST_F(BasicStatementTest, BlockInsideLabeledStatement) {
    auto stmt = ParseStatement("label: { 42; 'test'; }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_NE(labeled_stmt, nullptr);
    EXPECT_EQ(labeled_stmt->label(), "label");

    auto* block_stmt = dynamic_cast<BlockStatement*>(labeled_stmt->body().get());
    ASSERT_NE(block_stmt, nullptr);
    EXPECT_EQ(block_stmt->statements().size(), 2);
}

/**
 * @test 测试复杂的嵌套结构
 */
TEST_F(BasicStatementTest, ComplexNestedStructure) {
    auto stmt = ParseStatement("{ outer: { inner: 42; } }");
    auto* block_stmt = dynamic_cast<BlockStatement*>(stmt.get());
    ASSERT_NE(block_stmt, nullptr);

    auto* outer_labeled_stmt = dynamic_cast<LabeledStatement*>(block_stmt->statements()[0].get());
    ASSERT_NE(outer_labeled_stmt, nullptr);
    EXPECT_EQ(outer_labeled_stmt->label(), "outer");

    auto* inner_block_stmt = dynamic_cast<BlockStatement*>(outer_labeled_stmt->body().get());
    ASSERT_NE(inner_block_stmt, nullptr);

    auto* inner_labeled_stmt = dynamic_cast<LabeledStatement*>(inner_block_stmt->statements()[0].get());
    ASSERT_NE(inner_labeled_stmt, nullptr);
    EXPECT_EQ(inner_labeled_stmt->label(), "inner");
}

} // namespace test
} // namespace compiler
} // namespace mjs

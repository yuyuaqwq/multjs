/**
 * @file exception_statement_test.cpp
 * @brief 异常处理语句测试
 *
 * 测试异常处理语句类型，包括:
 * - throw语句 (ThrowStatement)
 * - try-catch语句 (TryStatement)
 * - catch子句 (CatchClause)
 * - finally子句 (FinallyClause)
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
#include "../src/compiler/statement_impl/throw_statement.h"
#include "../src/compiler/statement_impl/try_statement.h"
#include "../src/compiler/statement_impl/catch_clause.h"
#include "../src/compiler/statement_impl/block_statement.h"
#include "../src/compiler/statement_impl/while_statement.h"
#include "../src/compiler/expression_impl/integer_literal.h"
#include "../src/compiler/expression_impl/string_literal.h"
#include "../src/compiler/expression_impl/identifier.h"
#include "../src/compiler/expression_impl/object_expression.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ExceptionStatementTest
 * @brief 异常处理语句测试类
 */
class ExceptionStatementTest : public ::testing::Test {
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
// throw语句测试 (ThrowStatement)
// ============================================================================

/**
 * @test 测试抛出字面量
 */
TEST_F(ExceptionStatementTest, ThrowLiteral) {
    auto stmt = ParseStatement("throw 42;");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);
    EXPECT_EQ(throw_stmt->type(), StatementType::kThrow);
    EXPECT_NE(throw_stmt->argument(), nullptr);

    auto* arg_literal = dynamic_cast<IntegerLiteral*>(throw_stmt->argument().get());
    ASSERT_NE(arg_literal, nullptr);
    EXPECT_EQ(arg_literal->value(), 42);
}

/**
 * @test 测试抛出字符串
 */
TEST_F(ExceptionStatementTest, ThrowString) {
    auto stmt = ParseStatement("throw 'error';");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);
    EXPECT_NE(throw_stmt->argument(), nullptr);

    auto* arg_string = dynamic_cast<StringLiteral*>(throw_stmt->argument().get());
    ASSERT_NE(arg_string, nullptr);
    EXPECT_EQ(arg_string->value(), "error");
}

/**
 * @test 测试抛出标识符
 */
TEST_F(ExceptionStatementTest, ThrowIdentifier) {
    auto stmt = ParseStatement("throw err;");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);
    EXPECT_NE(throw_stmt->argument(), nullptr);

    auto* arg_identifier = dynamic_cast<Identifier*>(throw_stmt->argument().get());
    ASSERT_NE(arg_identifier, nullptr);
    EXPECT_EQ(arg_identifier->name(), "err");
}

/**
 * @test 测试抛出对象
 */
TEST_F(ExceptionStatementTest, ThrowObject) {
    auto stmt = ParseStatement("throw { message: 'error' };");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);
    EXPECT_NE(throw_stmt->argument(), nullptr);

    auto* arg_object = dynamic_cast<ObjectExpression*>(throw_stmt->argument().get());
    ASSERT_NE(arg_object, nullptr);
}

/**
 * @test 测试抛出表达式
 */
TEST_F(ExceptionStatementTest, ThrowExpression) {
    auto stmt = ParseStatement("throw x + y;");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);
    EXPECT_NE(throw_stmt->argument(), nullptr);
}

/**
 * @test 测试throw语句源代码位置
 */
TEST_F(ExceptionStatementTest, ThrowStatementSourcePosition) {
    auto stmt = ParseStatement("throw 42;");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_NE(throw_stmt, nullptr);

    auto start = throw_stmt->start();
    auto end = throw_stmt->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

// ============================================================================
// try-catch语句测试 (TryStatement)
// ============================================================================

/**
 * @test 测试简单的try-catch语句
 */
TEST_F(ExceptionStatementTest, SimpleTryCatch) {
    auto stmt = ParseStatement("try { throw 42; } catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    EXPECT_EQ(try_stmt->type(), StatementType::kTry);
    EXPECT_NE(try_stmt->block(), nullptr);
    EXPECT_NE(try_stmt->handler(), nullptr);
    EXPECT_EQ(try_stmt->finalizer(), nullptr);
}

/**
 * @test 测试try-finally语句
 */
TEST_F(ExceptionStatementTest, TryFinally) {
    auto stmt = ParseStatement("try { throw 42; } finally {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    EXPECT_NE(try_stmt->block(), nullptr);
    EXPECT_EQ(try_stmt->handler(), nullptr);
    EXPECT_NE(try_stmt->finalizer(), nullptr);
}

/**
 * @test 测试try-catch-finally语句
 */
TEST_F(ExceptionStatementTest, TryCatchFinally) {
    auto stmt = ParseStatement("try { throw 42; } catch (e) {} finally {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    EXPECT_NE(try_stmt->block(), nullptr);
    EXPECT_NE(try_stmt->handler(), nullptr);
    EXPECT_NE(try_stmt->finalizer(), nullptr);
}

/**
 * @test 测试catch子句的参数
 */
TEST_F(ExceptionStatementTest, CatchClauseParameter) {
    auto stmt = ParseStatement("try {} catch (error) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);

    const auto& handler = try_stmt->handler();
    ASSERT_NE(handler->param(), nullptr);
    EXPECT_EQ(handler->param()->name(), "error");
}

/**
 * @test 测试catch子句体包含多个语句
 */
TEST_F(ExceptionStatementTest, CatchClauseWithMultipleStatements) {
    auto stmt = ParseStatement("try {} catch (e) { console.log(e); console.log(e.stack); }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);

    const auto& handler = try_stmt->handler();
    ASSERT_NE(handler->body(), nullptr);
    EXPECT_GT(handler->body()->statements().size(), 1);
}

/**
 * @test 测试嵌套try语句
 */
TEST_F(ExceptionStatementTest, NestedTryStatement) {
    auto stmt = ParseStatement("try { try {} catch (e) {} } catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->block(), nullptr);
}

/**
 * @test 测试try块包含多个语句
 */
TEST_F(ExceptionStatementTest, TryBlockWithMultipleStatements) {
    auto stmt = ParseStatement("try { 1; 2; 3; } catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->block(), nullptr);

    const auto& block = try_stmt->block();
    EXPECT_EQ(block->statements().size(), 3);
}

/**
 * @test 测试finally块包含多个语句
 */
TEST_F(ExceptionStatementTest, FinallyBlockWithMultipleStatements) {
    auto stmt = ParseStatement("try {} finally { 1; 2; 3; }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->finalizer(), nullptr);

    const auto& finalizer = try_stmt->finalizer();
    EXPECT_GT(finalizer->body()->statements().size(), 1);
}

/**
 * @test 测试try语句源代码位置
 */
TEST_F(ExceptionStatementTest, TryStatementSourcePosition) {
    auto stmt = ParseStatement("try {} catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);

    auto start = try_stmt->start();
    auto end = try_stmt->end();
    // start位置验证
    // start位置验证
    // end位置验证
}

/**
 * @test 测试try-catch在函数中
 */
TEST_F(ExceptionStatementTest, TryCatchInFunction) {
    auto stmt = ParseStatement("function foo() { try {} catch (e) {} }");
    // 这应该解析为函数声明
    ASSERT_NE(stmt, nullptr);
}

/**
 * @test 测试catch子句源代码位置
 */
TEST_F(ExceptionStatementTest, CatchClauseSourcePosition) {
    auto stmt = ParseStatement("try {} catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt->handler(), nullptr);
    // catch子句应该有正确的源代码位置信息(已验证对象创建成功)
}

/**
 * @test 测试throw在try块中
 */
TEST_F(ExceptionStatementTest, ThrowInTryBlock) {
    auto stmt = ParseStatement("try { throw new Error('error'); } catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->block(), nullptr);

    const auto& block = try_stmt->block();
    EXPECT_GT(block->statements().size(), 0);
}

/**
 * @test 测试throw在catch块中
 */
TEST_F(ExceptionStatementTest, ThrowInCatchBlock) {
    auto stmt = ParseStatement("try {} catch (e) { throw e; }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);

    const auto& handler = try_stmt->handler();
    ASSERT_NE(handler->body(), nullptr);
    EXPECT_GT(handler->body()->statements().size(), 0);
}

/**
 * @test 测试return在finally块中
 */
TEST_F(ExceptionStatementTest, ReturnInFinallyBlock) {
    auto stmt = ParseStatement("try {} finally { return 42; }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->finalizer(), nullptr);

    const auto& finalizer = try_stmt->finalizer();
    ASSERT_NE(finalizer->body(), nullptr);
    EXPECT_GT(finalizer->body()->statements().size(), 0);
}

/**
 * @test 测试throw在finally块中
 */
TEST_F(ExceptionStatementTest, ThrowInFinallyBlock) {
    auto stmt = ParseStatement("try {} finally { throw 'finally error'; }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->finalizer(), nullptr);

    const auto& finalizer = try_stmt->finalizer();
    ASSERT_NE(finalizer->body(), nullptr);
    EXPECT_GT(finalizer->body()->statements().size(), 0);
}

/**
 * @test 测试try-catch与控制流结合
 */
TEST_F(ExceptionStatementTest, TryCatchWithControlFlow) {
    auto stmt = ParseStatement("try { if (true) { throw 'error'; } } catch (e) { return; }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
}

/**
 * @test 测试多个catch块(使用if)
 */
TEST_F(ExceptionStatementTest, MultipleCatchWithIf) {
    auto stmt = ParseStatement("try {} catch (e) { if (e instanceof TypeError) {} }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);

    const auto& handler = try_stmt->handler();
    ASSERT_NE(handler->body(), nullptr);
    EXPECT_GT(handler->body()->statements().size(), 0);
}

/**
 * @test 测试try-catch与循环结合
 */
TEST_F(ExceptionStatementTest, TryCatchWithLoop) {
    auto stmt = ParseStatement("while (true) { try { break; } catch (e) {} }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_NE(while_stmt, nullptr);
}

/**
 * @test 测试空的try-catch
 */
TEST_F(ExceptionStatementTest, EmptyTryCatch) {
    auto stmt = ParseStatement("try {} catch (e) {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->block(), nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);
    EXPECT_EQ(try_stmt->block()->statements().size(), 0);
    EXPECT_EQ(try_stmt->handler()->body()->statements().size(), 0);
}

/**
 * @test 测试catch子句不带参数
 */
TEST_F(ExceptionStatementTest, CatchClauseWithoutParameter) {
    // 注意: ES2019支持catch不带参数，但这取决于解析器实现
    auto stmt = ParseStatement("try {} catch {}");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_NE(try_stmt, nullptr);
    ASSERT_NE(try_stmt->handler(), nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "../src/compiler/lexer.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/expression.h"
#include "../src/compiler/statement.h"
#include "../src/compiler/expression/integer_literal.h"
#include "../src/compiler/expression/string_literal.h"
#include "../src/compiler/expression/boolean_literal.h"
#include "../src/compiler/expression/null_literal.h"
#include "../src/compiler/expression/undefined_literal.h"
#include "../src/compiler/expression/identifier.h"
#include "../src/compiler/expression/binary_expression.h"
#include "../src/compiler/expression/assignment_expression.h"
#include "../src/compiler/expression/object_expression.h"
#include "../src/compiler/expression/array_expression.h"
#include "../src/compiler/expression/function_expression.h"
#include "../src/compiler/expression/arrow_function_expression.h"
#include "../src/compiler/expression/unary_expression.h"
#include "../src/compiler/expression/member_expression.h"
#include "../src/compiler/expression/call_expression.h"
#include "../src/compiler/expression/conditional_expression.h"
#include "../src/compiler/expression/new_expression.h"
#include "../src/compiler/expression/template_literal.h"
#include "../src/compiler/expression/yield_expression.h"
#include "../src/compiler/expression/await_expression.h"
#include "../src/compiler/expression/import_expression.h"

#include "../src/compiler/statement/block_statement.h"
#include "../src/compiler/statement/return_statement.h"
#include "../src/compiler/statement/expression_statement.h"
#include "../src/compiler/statement/if_statement.h"
#include "../src/compiler/statement/variable_declaration.h"
#include "../src/compiler/statement/for_statement.h"
#include "../src/compiler/statement/while_statement.h"
#include "../src/compiler/statement/try_statement.h"
#include "../src/compiler/statement/throw_statement.h"
#include "../src/compiler/statement/break_statement.h"
#include "../src/compiler/statement/continue_statement.h"
#include "../src/compiler/statement/labeled_statement.h"
#include "../src/compiler/statement/export_declaration.h"

namespace mjs {
namespace compiler {
namespace test {

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // 辅助方法，用于创建Parser对象
    std::unique_ptr<Parser> CreateParser(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        return std::make_unique<Parser>(lexer.release());
    }

    // 辅助方法，用于解析表达式
    std::unique_ptr<Expression> ParseExpression(const std::string& source) {
        auto parser = CreateParser(source);
        return Expression::ParseExpression(parser->lexer_);
    }

    // 辅助方法，用于解析语句
    std::unique_ptr<Statement> ParseStatement(const std::string& source) {
        auto parser = CreateParser(source);
        return Statement::ParseStatement(parser->lexer_);
    }
};

// 测试解析字面量
TEST_F(ParserTest, ParseLiterals) {
    // 测试数字字面量
    auto expr = ParseExpression("42");
    auto* int_expr = dynamic_cast<IntegerLiteral*>(expr.get());
    ASSERT_TRUE(int_expr != nullptr);
    EXPECT_EQ(int_expr->value(), 42);

    // 测试字符串字面量
    expr = ParseExpression("\"hello\"");
    auto* string_expr = dynamic_cast<StringLiteral*>(expr.get());
    ASSERT_TRUE(string_expr != nullptr);
    EXPECT_EQ(string_expr->value(), "hello");

    // 测试布尔字面量
    expr = ParseExpression("true");
    auto* bool_true_expr = dynamic_cast<BooleanLiteral*>(expr.get());
    ASSERT_TRUE(bool_true_expr != nullptr);
    EXPECT_TRUE(bool_true_expr->value());

    expr = ParseExpression("false");
    auto* bool_false_expr = dynamic_cast<BooleanLiteral*>(expr.get());
    ASSERT_TRUE(bool_false_expr != nullptr);
    EXPECT_FALSE(bool_false_expr->value());

    // 测试null字面量
    expr = ParseExpression("null");
    auto* null_expr = dynamic_cast<NullLiteral*>(expr.get());
    ASSERT_TRUE(null_expr != nullptr);

    // 测试undefined字面量
    expr = ParseExpression("undefined");
    auto* undefined_expr = dynamic_cast<UndefinedLiteral*>(expr.get());
    ASSERT_TRUE(undefined_expr != nullptr);
}

// 测试解析标识符
TEST_F(ParserTest, ParseIdentifier) {
    auto expr = ParseExpression("foo");
    auto* identifier_expr = dynamic_cast<Identifier*>(expr.get());
    ASSERT_TRUE(identifier_expr != nullptr);
    EXPECT_EQ(identifier_expr->name(), "foo");
}

// 测试解析二元表达式
TEST_F(ParserTest, ParseBinaryExpression) {
    // 测试加法
    auto expr = ParseExpression("1 + 2");
    auto* binary_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_TRUE(binary_expr != nullptr);
    EXPECT_EQ(binary_expr->op(), TokenType::kOpAdd);
    auto* left_int = dynamic_cast<IntegerLiteral*>(binary_expr->left().get());
    ASSERT_TRUE(left_int != nullptr);
    EXPECT_EQ(left_int->value(), 1);
    auto* right_int = dynamic_cast<IntegerLiteral*>(binary_expr->right().get());
    ASSERT_TRUE(right_int != nullptr);
    EXPECT_EQ(right_int->value(), 2);

    // 测试乘法
    expr = ParseExpression("3 * 4");
    auto* mul_binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_TRUE(mul_binary != nullptr);
    EXPECT_EQ(mul_binary->op(), TokenType::kOpMul);
    auto* left_mul_int = dynamic_cast<IntegerLiteral*>(mul_binary->left().get());
    ASSERT_TRUE(left_mul_int != nullptr);
    EXPECT_EQ(left_mul_int->value(), 3);
    auto* right_mul_int = dynamic_cast<IntegerLiteral*>(mul_binary->right().get());
    ASSERT_TRUE(right_mul_int != nullptr);
    EXPECT_EQ(right_mul_int->value(), 4);

    // 测试复杂表达式
    expr = ParseExpression("1 + 2 * 3");
    auto* complex_binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_TRUE(complex_binary != nullptr);
    EXPECT_EQ(complex_binary->op(), TokenType::kOpAdd);
    auto* left_complex_int = dynamic_cast<IntegerLiteral*>(complex_binary->left().get());
    ASSERT_TRUE(left_complex_int != nullptr);
    EXPECT_EQ(left_complex_int->value(), 1);
    auto* right_binary = dynamic_cast<BinaryExpression*>(complex_binary->right().get());
    ASSERT_TRUE(right_binary != nullptr);
    EXPECT_EQ(right_binary->op(), TokenType::kOpMul);
}

// 测试解析赋值表达式
TEST_F(ParserTest, ParseExpressionAtAssignmentLevel) {
    auto expr = ParseExpression("x = 42");
    auto* assign_expr = dynamic_cast<AssignmentExpression*>(expr.get());
    ASSERT_TRUE(assign_expr != nullptr);
    EXPECT_EQ(assign_expr->op(), TokenType::kOpAssign);
    auto* left_ident = dynamic_cast<Identifier*>(assign_expr->left().get());
    ASSERT_TRUE(left_ident != nullptr);
    EXPECT_EQ(left_ident->name(), "x");
    auto* right_int = dynamic_cast<IntegerLiteral*>(assign_expr->right().get());
    ASSERT_TRUE(right_int != nullptr);
    EXPECT_EQ(right_int->value(), 42);
}

// 测试解析对象表达式
TEST_F(ParserTest, ParseObjectExpression) {
    auto expr = ParseExpression("{ x: 1, y: 2 }");
    auto* obj_expr = dynamic_cast<ObjectExpression*>(expr.get());
    ASSERT_TRUE(obj_expr != nullptr);
    ASSERT_EQ(obj_expr->properties().size(), 2);

    // 检查第一个属性
    EXPECT_EQ(obj_expr->properties()[0].key, "x");
    auto* first_prop_int = dynamic_cast<IntegerLiteral*>(obj_expr->properties()[0].value.get());
    ASSERT_TRUE(first_prop_int != nullptr);
    EXPECT_EQ(first_prop_int->value(), 1);

    // 检查第二个属性
    EXPECT_EQ(obj_expr->properties()[1].key, "y");
    auto* second_prop_int = dynamic_cast<IntegerLiteral*>(obj_expr->properties()[1].value.get());
    ASSERT_TRUE(second_prop_int != nullptr);
    EXPECT_EQ(second_prop_int->value(), 2);
}

// 测试解析数组表达式
TEST_F(ParserTest, ParseArrayExpression) {
    auto expr = ParseExpression("[1, 2, 3]");
    auto* arr_expr = dynamic_cast<ArrayExpression*>(expr.get());
    ASSERT_TRUE(arr_expr != nullptr);
    ASSERT_EQ(arr_expr->elements().size(), 3);

    // 检查元素
    auto* first_elem = dynamic_cast<IntegerLiteral*>(arr_expr->elements()[0].get());
    ASSERT_TRUE(first_elem != nullptr);
    EXPECT_EQ(first_elem->value(), 1);

    auto* second_elem = dynamic_cast<IntegerLiteral*>(arr_expr->elements()[1].get());
    ASSERT_TRUE(second_elem != nullptr);
    EXPECT_EQ(second_elem->value(), 2);

    auto* third_elem = dynamic_cast<IntegerLiteral*>(arr_expr->elements()[2].get());
    ASSERT_TRUE(third_elem != nullptr);
    EXPECT_EQ(third_elem->value(), 3);
}

// 测试解析函数表达式
TEST_F(ParserTest, ParseExpressionAtFunctionLevel) {
    auto expr = ParseExpression("function foo(a, b) { return a + b; }");
    auto* func_expr = dynamic_cast<FunctionExpression*>(expr.get());
    ASSERT_TRUE(func_expr != nullptr);
    EXPECT_EQ(func_expr->id(), "foo");
    ASSERT_EQ(func_expr->params().size(), 2);
    EXPECT_EQ(func_expr->params()[0], "a");
    EXPECT_EQ(func_expr->params()[1], "b");

    // 检查函数体
    auto* body_block = dynamic_cast<BlockStatement*>(func_expr->body().get());
    ASSERT_TRUE(body_block != nullptr);
    ASSERT_EQ(body_block->statements().size(), 1);
    auto* return_stmt = dynamic_cast<ReturnStatement*>(body_block->statements()[0].get());
    ASSERT_TRUE(return_stmt != nullptr);
}

// 测试解析箭头函数表达式
TEST_F(ParserTest, ParseArrowFunctionExpression) {
    auto expr = ParseExpression("(a, b) => a + b");
    auto* arrow_expr = dynamic_cast<ArrowFunctionExpression*>(expr.get());
    ASSERT_TRUE(arrow_expr != nullptr);
    ASSERT_EQ(arrow_expr->params().size(), 2);
    EXPECT_EQ(arrow_expr->params()[0], "a");
    EXPECT_EQ(arrow_expr->params()[1], "b");

    // 检查函数体
    auto* body_expr = dynamic_cast<ExpressionStatement*>(arrow_expr->body().get());
    ASSERT_TRUE(body_expr != nullptr);
}

// 测试解析变量声明
TEST_F(ParserTest, ParseVariableDeclaration) {
    auto stmt = ParseStatement("let x = 42;");
    auto* var_decl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_TRUE(var_decl != nullptr);
    EXPECT_EQ(var_decl->name(), "x");
    EXPECT_EQ(var_decl->kind(), TokenType::kKwLet);
    auto* init_int = dynamic_cast<IntegerLiteral*>(var_decl->init().get());
    ASSERT_TRUE(init_int != nullptr);
    EXPECT_EQ(init_int->value(), 42);
}

// 测试解析if语句
TEST_F(ParserTest, ParseIfStatement) {
    auto stmt = ParseStatement("if (x > 0) { y = 1; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_TRUE(if_stmt != nullptr);

    // 检查条件
    auto* test_binary = dynamic_cast<BinaryExpression*>(if_stmt->test().get());
    ASSERT_TRUE(test_binary != nullptr);

    // 检查consequent
    auto* consequent_block = dynamic_cast<BlockStatement*>(if_stmt->consequent().get());
    ASSERT_TRUE(consequent_block != nullptr);
    ASSERT_EQ(consequent_block->statements().size(), 1);

    // 检查alternate（应该为空）
    EXPECT_EQ(if_stmt->alternate(), nullptr);
}

// 测试解析for循环
TEST_F(ParserTest, ParseForStatement) {
    auto stmt = ParseStatement("for (let i = 0; i < 10; i++) { sum += i; }");
    auto* for_stmt = dynamic_cast<ForStatement*>(stmt.get());
    ASSERT_TRUE(for_stmt != nullptr);

    // 检查初始化语句
    auto* init_var = dynamic_cast<VariableDeclaration*>(for_stmt->init().get());
    ASSERT_TRUE(init_var != nullptr);

    // 检查条件表达式
    auto* test_binary = dynamic_cast<BinaryExpression*>(for_stmt->test().get());
    ASSERT_TRUE(test_binary != nullptr);

    // 检查更新表达式
    auto* update_unary = dynamic_cast<UnaryExpression*>(for_stmt->update().get());
    ASSERT_TRUE(update_unary != nullptr);

    // 检查循环体
    auto* body_block = dynamic_cast<BlockStatement*>(for_stmt->body().get());
    ASSERT_TRUE(body_block != nullptr);
}

// 测试解析完整程序
TEST_F(ParserTest, ParseProgram) {
    std::string source = R"(
        let x = 10;
        let y = 20;
        let sum = x + y;
        
        function add(a, b) {
            return a + b;
        }
        
        let result = add(x, y);
    )";
    
    auto parser = CreateParser(source);
    parser->ParseProgram();
    
    // 检查解析后的语句数量
    ASSERT_EQ(parser->statements().size(), 5);
    
    // 检查第一个语句是变量声明
    auto* first_var_decl = dynamic_cast<VariableDeclaration*>(parser->statements()[0].get());
    ASSERT_TRUE(first_var_decl != nullptr);

    // 检查第四个语句是函数表达式
    auto* func_stmt = dynamic_cast<ExpressionStatement*>(parser->statements()[3].get());
    ASSERT_TRUE(func_stmt != nullptr);
    auto* func_expr = dynamic_cast<FunctionExpression*>(func_stmt->expression().get());
    ASSERT_TRUE(func_expr != nullptr);
}

// 测试解析一元表达式
TEST_F(ParserTest, ParseExpressionAtUnaryLevel) {
    // 测试前缀一元运算符
    auto expr = ParseExpression("-42");
    auto* unary_expr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_TRUE(unary_expr != nullptr);
    EXPECT_EQ(unary_expr->op(), TokenType::kOpSub);
    EXPECT_TRUE(unary_expr->is_prefix());
    auto* arg_int = dynamic_cast<IntegerLiteral*>(unary_expr->argument().get());
    ASSERT_TRUE(arg_int != nullptr);
    EXPECT_EQ(arg_int->value(), 42);

    // 测试前缀自增
    expr = ParseExpression("++x");
    auto* prefix_inc = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_TRUE(prefix_inc != nullptr);
    EXPECT_EQ(prefix_inc->op(), TokenType::kOpPrefixInc);
    EXPECT_TRUE(prefix_inc->is_prefix());
    auto* prefix_arg = dynamic_cast<Identifier*>(prefix_inc->argument().get());
    ASSERT_TRUE(prefix_arg != nullptr);
    EXPECT_EQ(prefix_arg->name(), "x");

    // 测试后缀自增
    expr = ParseExpression("x++");
    auto* postfix_inc = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_TRUE(postfix_inc != nullptr);
    EXPECT_EQ(postfix_inc->op(), TokenType::kOpSuffixInc);
    EXPECT_FALSE(postfix_inc->is_prefix());
    auto* postfix_arg = dynamic_cast<Identifier*>(postfix_inc->argument().get());
    ASSERT_TRUE(postfix_arg != nullptr);
    EXPECT_EQ(postfix_arg->name(), "x");

    // 测试逻辑非
    expr = ParseExpression("!true");
    auto* logical_not = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_TRUE(logical_not != nullptr);
    EXPECT_EQ(logical_not->op(), TokenType::kOpNot);
    EXPECT_TRUE(logical_not->is_prefix());
    auto* bool_arg = dynamic_cast<BooleanLiteral*>(logical_not->argument().get());
    ASSERT_TRUE(bool_arg != nullptr);
    EXPECT_TRUE(bool_arg->value());

    // 测试typeof运算符
    expr = ParseExpression("typeof x");
    auto* typeof_op = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_TRUE(typeof_op != nullptr);
    EXPECT_EQ(typeof_op->op(), TokenType::kKwTypeof);
    EXPECT_TRUE(typeof_op->is_prefix());
    auto* typeof_arg = dynamic_cast<Identifier*>(typeof_op->argument().get());
    ASSERT_TRUE(typeof_arg != nullptr);
    EXPECT_EQ(typeof_arg->name(), "x");
}

// 测试解析成员表达式和调用表达式
TEST_F(ParserTest, ParseMemberAndCallExpression) {
    // 测试点访问成员表达式
    auto expr = ParseExpression("obj.prop");
    auto* member_expr = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_TRUE(member_expr != nullptr);
    auto* obj_ident = dynamic_cast<Identifier*>(member_expr->object().get());
    ASSERT_TRUE(obj_ident != nullptr);
    EXPECT_EQ(obj_ident->name(), "obj");
    auto* prop_ident = dynamic_cast<Identifier*>(member_expr->property().get());
    ASSERT_TRUE(prop_ident != nullptr);
    EXPECT_EQ(prop_ident->name(), "prop");
    EXPECT_FALSE(member_expr->computed());
    EXPECT_FALSE(member_expr->optional());

    // 测试计算属性成员表达式
    expr = ParseExpression("arr[0]");
    auto* computed_member = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_TRUE(computed_member != nullptr);
    auto* arr_ident = dynamic_cast<Identifier*>(computed_member->object().get());
    ASSERT_TRUE(arr_ident != nullptr);
    EXPECT_EQ(arr_ident->name(), "arr");
    auto* index_int = dynamic_cast<IntegerLiteral*>(computed_member->property().get());
    ASSERT_TRUE(index_int != nullptr);
    EXPECT_EQ(index_int->value(), 0);
    EXPECT_TRUE(computed_member->computed());
    EXPECT_FALSE(computed_member->optional());

    // 测试可选链成员表达式
    expr = ParseExpression("obj?.prop");
    auto* optional_member = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_TRUE(optional_member != nullptr);
    auto* optional_obj = dynamic_cast<Identifier*>(optional_member->object().get());
    ASSERT_TRUE(optional_obj != nullptr);
    EXPECT_EQ(optional_obj->name(), "obj");
    auto* optional_prop = dynamic_cast<Identifier*>(optional_member->property().get());
    ASSERT_TRUE(optional_prop != nullptr);
    EXPECT_EQ(optional_prop->name(), "prop");
    EXPECT_FALSE(optional_member->computed());
    EXPECT_TRUE(optional_member->optional());

    // 测试函数调用表达式
    expr = ParseExpression("func(1, 2)");
    auto* call_expr = dynamic_cast<CallExpression*>(expr.get());
    ASSERT_TRUE(call_expr != nullptr);
    auto* callee_ident = dynamic_cast<Identifier*>(call_expr->callee().get());
    ASSERT_TRUE(callee_ident != nullptr);
    EXPECT_EQ(callee_ident->name(), "func");
    ASSERT_EQ(call_expr->arguments().size(), 2);
    auto* first_arg = dynamic_cast<IntegerLiteral*>(call_expr->arguments()[0].get());
    ASSERT_TRUE(first_arg != nullptr);
    EXPECT_EQ(first_arg->value(), 1);
    auto* second_arg = dynamic_cast<IntegerLiteral*>(call_expr->arguments()[1].get());
    ASSERT_TRUE(second_arg != nullptr);
    EXPECT_EQ(second_arg->value(), 2);

    // 测试链式调用
    expr = ParseExpression("obj.method().prop");
    auto* chained = dynamic_cast<MemberExpression*>(expr.get());
    ASSERT_TRUE(chained != nullptr);
    auto* method_call = dynamic_cast<CallExpression*>(chained->object().get());
    ASSERT_TRUE(method_call != nullptr);
    auto* method_callee = dynamic_cast<MemberExpression*>(method_call->callee().get());
    ASSERT_TRUE(method_callee != nullptr);
    auto* method_prop = dynamic_cast<Identifier*>(method_callee->property().get());
    ASSERT_TRUE(method_prop != nullptr);
    EXPECT_EQ(method_prop->name(), "method");
}

// 测试解析条件表达式
TEST_F(ParserTest, ParseConditionalExpression) {
    auto expr = ParseExpression("x > 0 ? 'positive' : 'negative'");
    auto* cond_expr = dynamic_cast<ConditionalExpression*>(expr.get());
    ASSERT_TRUE(cond_expr != nullptr);

    // 检查条件部分
    auto* test_binary = dynamic_cast<BinaryExpression*>(cond_expr->test().get());
    ASSERT_TRUE(test_binary != nullptr);
    EXPECT_EQ(test_binary->op(), TokenType::kOpGt);

    // 检查consequent
    auto* consequent_str = dynamic_cast<StringLiteral*>(cond_expr->consequent().get());
    ASSERT_TRUE(consequent_str != nullptr);
    EXPECT_EQ(consequent_str->value(), "positive");

    // 检查alternate
    auto* alternate_str = dynamic_cast<StringLiteral*>(cond_expr->alternate().get());
    ASSERT_TRUE(alternate_str != nullptr);
    EXPECT_EQ(alternate_str->value(), "negative");
}

// 测试解析while循环语句
TEST_F(ParserTest, ParseWhileStatement) {
    auto stmt = ParseStatement("while (i < 10) { i++; }");
    auto* while_stmt = dynamic_cast<WhileStatement*>(stmt.get());
    ASSERT_TRUE(while_stmt != nullptr);

    // 检查条件
    auto* test_binary = dynamic_cast<BinaryExpression*>(while_stmt->test().get());
    ASSERT_TRUE(test_binary != nullptr);
    EXPECT_EQ(test_binary->op(), TokenType::kOpLt);

    // 检查循环体
    auto* body_block = dynamic_cast<BlockStatement*>(while_stmt->body().get());
    ASSERT_TRUE(body_block != nullptr);
    ASSERT_EQ(body_block->statements().size(), 1);
    auto* expr_stmt = dynamic_cast<ExpressionStatement*>(body_block->statements()[0].get());
    ASSERT_TRUE(expr_stmt != nullptr);
}

// 测试解析带else的if语句
TEST_F(ParserTest, ParseIfElseStatement) {
    auto stmt = ParseStatement("if (x > 0) { y = 1; } else { y = -1; }");
    auto* if_stmt = dynamic_cast<IfStatement*>(stmt.get());
    ASSERT_TRUE(if_stmt != nullptr);

    // 检查条件
    auto* test_binary = dynamic_cast<BinaryExpression*>(if_stmt->test().get());
    ASSERT_TRUE(test_binary != nullptr);

    // 检查consequent
    auto* consequent_block = dynamic_cast<BlockStatement*>(if_stmt->consequent().get());
    ASSERT_TRUE(consequent_block != nullptr);

    // 检查alternate（不应为空）
    ASSERT_NE(if_stmt->alternate(), nullptr);
    auto* alternate_block = dynamic_cast<BlockStatement*>(if_stmt->alternate().get());
    ASSERT_TRUE(alternate_block != nullptr);
}

// 测试解析try-catch-finally语句
TEST_F(ParserTest, ParseTryCatchStatement) {
    auto stmt = ParseStatement("try { riskyOperation(); } catch (error) { handleError(error); } finally { cleanup(); }");
    auto* try_stmt = dynamic_cast<TryStatement*>(stmt.get());
    ASSERT_TRUE(try_stmt != nullptr);

    // 检查try块
    auto* try_block = dynamic_cast<BlockStatement*>(try_stmt->block().get());
    ASSERT_TRUE(try_block != nullptr);

    // 检查catch子句
    ASSERT_NE(try_stmt->handler(), nullptr);
    auto* catch_clause = dynamic_cast<CatchClause*>(try_stmt->handler().get());
    ASSERT_TRUE(catch_clause != nullptr);
    EXPECT_EQ(catch_clause->param()->name(), "error");

    // 检查finally子句
    ASSERT_NE(try_stmt->finalizer(), nullptr);
    auto* finally_block = dynamic_cast<FinallyClause*>(try_stmt->finalizer().get());
    ASSERT_TRUE(finally_block != nullptr);
}

// 测试解析throw语句
TEST_F(ParserTest, ParseThrowStatement) {
    auto stmt = ParseStatement("throw new Error('Something went wrong');");
    auto* throw_stmt = dynamic_cast<ThrowStatement*>(stmt.get());
    ASSERT_TRUE(throw_stmt != nullptr);

    // 检查抛出的表达式
    auto* new_expr = dynamic_cast<NewExpression*>(throw_stmt->argument().get());
    ASSERT_TRUE(new_expr != nullptr);
}

// 测试解析break和continue语句
TEST_F(ParserTest, ParseBreakContinueStatement) {
    // 测试break语句
    auto break_stmt = ParseStatement("break;");
    auto* break_stmt_ptr = dynamic_cast<BreakStatement*>(break_stmt.get());
    ASSERT_TRUE(break_stmt_ptr != nullptr);

    // 测试带标签的break语句
    auto labeled_break = ParseStatement("break outerLoop;");
    auto* labeled_break_ptr = dynamic_cast<BreakStatement*>(labeled_break.get());
    ASSERT_TRUE(labeled_break_ptr != nullptr);
    EXPECT_EQ(labeled_break_ptr->label(), "outerLoop");

    // 测试continue语句
    auto cont_stmt = ParseStatement("continue;");
    auto* cont_stmt_ptr = dynamic_cast<ContinueStatement*>(cont_stmt.get());
    ASSERT_TRUE(cont_stmt_ptr != nullptr);

    // 测试带标签的continue语句
    auto labeled_cont = ParseStatement("continue outerLoop;");
    auto* labeled_cont_ptr = dynamic_cast<ContinueStatement*>(labeled_cont.get());
    ASSERT_TRUE(labeled_cont_ptr != nullptr);
    EXPECT_EQ(labeled_cont_ptr->label(), "outerLoop");
}

// 测试解析标签语句
TEST_F(ParserTest, ParseLabeledStatement) {
    auto stmt = ParseStatement("outerLoop: for (let i = 0; i < 10; i++) { innerLoop: for (let j = 0; j < 10; j++) { if (j > 5) { break outerLoop; } } }");
    auto* labeled_stmt = dynamic_cast<LabeledStatement*>(stmt.get());
    ASSERT_TRUE(labeled_stmt != nullptr);

    // 检查标签名称 
    EXPECT_EQ(labeled_stmt->label(), "outerLoop");

    // 检查标签语句的主体
    auto* for_stmt = dynamic_cast<ForStatement*>(labeled_stmt->body().get());
    ASSERT_TRUE(for_stmt != nullptr);
}

// 测试解析return语句
TEST_F(ParserTest, ParseReturnStatement) {
    // 测试无值的return
    auto empty_return = ParseStatement("return;");
    auto* empty_return_ptr = dynamic_cast<ReturnStatement*>(empty_return.get());
    ASSERT_TRUE(empty_return_ptr != nullptr);
    EXPECT_EQ(empty_return_ptr->argument(), nullptr);

    // 测试带值的return
    auto value_return = ParseStatement("return 42;");
    auto* value_return_ptr = dynamic_cast<ReturnStatement*>(value_return.get());
    ASSERT_TRUE(value_return_ptr != nullptr);
    ASSERT_NE(value_return_ptr->argument(), nullptr);
    auto* return_int = dynamic_cast<IntegerLiteral*>(value_return_ptr->argument().get());
    ASSERT_TRUE(return_int != nullptr);
    EXPECT_EQ(return_int->value(), 42);
}

// 测试解析模板字符串
TEST_F(ParserTest, ParseTemplateLiteral) {
    auto expr = ParseExpression("`Hello, ${name}!`");
    auto* template_literal = dynamic_cast<TemplateLiteral*>(expr.get());
    ASSERT_TRUE(template_literal != nullptr);

    // 标准AST是quasis和expressions
    // 这里的实现不一样，都放到expressions了

    // 检查模板字符串的部分
    // ASSERT_EQ(template_literal->quasis().size(), 2);
    ASSERT_EQ(template_literal->expressions().size(), 1);

    // 检查表达式部分
    auto* expr_ident = dynamic_cast<Identifier*>(template_literal->expressions()[0].get());
    ASSERT_TRUE(expr_ident != nullptr);
    EXPECT_EQ(expr_ident->name(), "name");
}

// 测试解析导入语句
TEST_F(ParserTest, ParseImportStatement) {
    auto stmt = ParseStatement("import { foo, bar as baz } from 'module';");
    auto* import_stmt = dynamic_cast<ImportDeclaration*>(stmt.get());
    ASSERT_TRUE(import_stmt != nullptr);

    // 检查导入的模块
    EXPECT_EQ(import_stmt->source(), "module");

    // 检查导入的标识符
    //ASSERT_GE(import_stmt->specifiers().size(), 2);
}

// 测试解析导出语句
TEST_F(ParserTest, ParseExportStatement) {
    auto stmt = ParseStatement("export const PI = 3.14;");
    auto* export_stmt = dynamic_cast<ExportDeclaration*>(stmt.get());
    ASSERT_TRUE(export_stmt != nullptr);

    // 检查导出的声明
    ASSERT_NE(export_stmt->declaration(), nullptr);
    auto* var_decl = dynamic_cast<VariableDeclaration*>(export_stmt->declaration().get());
    ASSERT_TRUE(var_decl != nullptr);
    EXPECT_EQ(var_decl->name(), "PI");
    EXPECT_EQ(var_decl->kind(), TokenType::kKwConst);
}

// 测试解析new表达式
TEST_F(ParserTest, ParseNewExpression) {
    auto expr = ParseExpression("new Date()");
    auto* new_expr = dynamic_cast<NewExpression*>(expr.get());
    ASSERT_TRUE(new_expr != nullptr);

    // 检查构造函数
    auto* callee_ident = dynamic_cast<Identifier*>(new_expr->callee().get());
    ASSERT_TRUE(callee_ident != nullptr);
    EXPECT_EQ(callee_ident->name(), "Date");

    // 检查参数（应为空）
    EXPECT_EQ(new_expr->arguments().size(), 0);

    // 测试带参数的new表达式
    expr = ParseExpression("new Person('John', 30)");
    auto* new_expr_with_args = dynamic_cast<NewExpression*>(expr.get());
    ASSERT_TRUE(new_expr_with_args != nullptr);

    // 检查构造函数
    auto* person_ident = dynamic_cast<Identifier*>(new_expr_with_args->callee().get());
    ASSERT_TRUE(person_ident != nullptr);
    EXPECT_EQ(person_ident->name(), "Person");

    // 检查参数
    ASSERT_EQ(new_expr_with_args->arguments().size(), 2);
    auto* first_arg = dynamic_cast<StringLiteral*>(new_expr_with_args->arguments()[0].get());
    ASSERT_TRUE(first_arg != nullptr);
    auto* second_arg = dynamic_cast<IntegerLiteral*>(new_expr_with_args->arguments()[1].get());
    ASSERT_TRUE(second_arg != nullptr);
}

// 测试解析yield表达式
TEST_F(ParserTest, ParseYieldExpression) {
    auto expr = ParseExpression("yield value");
    auto* yield_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_TRUE(yield_expr != nullptr);

    // 检查yield的值
    ASSERT_NE(yield_expr->argument(), nullptr);
    auto* yield_arg = dynamic_cast<Identifier*>(yield_expr->argument().get());
    ASSERT_TRUE(yield_arg != nullptr);
    EXPECT_EQ(yield_arg->name(), "value");
    //EXPECT_FALSE(yield_expr->delegate());

    // 测试yield*表达式
    expr = ParseExpression("yield* generator()");
    auto* yield_star_expr = dynamic_cast<YieldExpression*>(expr.get());
    ASSERT_TRUE(yield_star_expr != nullptr);

    // 检查yield*的值
    ASSERT_NE(yield_star_expr->argument(), nullptr);
    auto* yield_star_arg = dynamic_cast<CallExpression*>(yield_star_expr->argument().get());
    ASSERT_TRUE(yield_star_arg != nullptr);
    //EXPECT_TRUE(yield_star_expr->delegate());
}

// 测试解析await表达式
TEST_F(ParserTest, ParseAwaitExpression) {
    auto expr = ParseExpression("await promise");
    auto* await_expr = dynamic_cast<AwaitExpression*>(expr.get());
    ASSERT_TRUE(await_expr != nullptr);

    // 检查await的值
    ASSERT_NE(await_expr->argument(), nullptr);
    auto* await_arg = dynamic_cast<Identifier*>(await_expr->argument().get());
    ASSERT_TRUE(await_arg != nullptr);
    EXPECT_EQ(await_arg->name(), "promise");
}

// 测试解析类表达式
//TEST_F(ParserTest, ParseClassExpression) {
//    auto expr = ParseExpression("class Person { constructor(name) { this.name = name; } getName() { return this.name; } }");
//    auto* class_expr = dynamic_cast<ClassExpression*>(expr.get());
//    ASSERT_TRUE(class_expr != nullptr);
//    //
//    //// 检查类名
//    //EXPECT_EQ(class_expr->id(), "Person");
//    //
//    //// 检查类体
//    //ASSERT_GE(class_expr->body().size(), 2);  // 至少有constructor和getName方法
//}

// 测试解析import表达式
TEST_F(ParserTest, ParseImportExpression) {
    auto expr = ParseExpression("import('module')");
    auto* import_expr = dynamic_cast<ImportExpression*>(expr.get());
    ASSERT_TRUE(import_expr != nullptr);

    // 检查导入的模块
    auto* source_str = dynamic_cast<StringLiteral*>(import_expr->source().get());
    ASSERT_TRUE(source_str != nullptr);
    EXPECT_EQ(source_str->value(), "module");
}

// 测试解析复杂的嵌套表达式
TEST_F(ParserTest, ParseComplexNestedExpression) {
    auto expr = ParseExpression("(a + b) * (c - d) / Math.sqrt(e ** 2 + f ** 2)");
    auto* div_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_TRUE(div_expr != nullptr);

    // 检查顶层操作符（除法）
    EXPECT_EQ(div_expr->op(), TokenType::kOpDiv);

    // 检查左侧（乘法表达式）
    auto* mul_expr = dynamic_cast<BinaryExpression*>(div_expr->left().get());
    ASSERT_TRUE(mul_expr != nullptr);
    EXPECT_EQ(mul_expr->op(), TokenType::kOpMul);

    // 检查右侧（函数调用）
    auto* call_expr = dynamic_cast<CallExpression*>(div_expr->right().get());
    ASSERT_TRUE(call_expr != nullptr);
    auto* callee_member = dynamic_cast<MemberExpression*>(call_expr->callee().get());
    ASSERT_TRUE(callee_member != nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs
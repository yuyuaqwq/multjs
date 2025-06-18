#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "../src/compiler/lexer.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/expression.h"
#include "../src/compiler/statement.h"

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
        return parser->ParseExpression();
    }

    // 辅助方法，用于解析语句
    std::unique_ptr<Statement> ParseStatement(const std::string& source) {
        auto parser = CreateParser(source);
        return parser->ParseStatement();
    }
};

// 测试解析字面量
TEST_F(ParserTest, ParseLiterals) {
    // 测试数字字面量
    auto expr = ParseExpression("42");
    ASSERT_TRUE(expr->is(ExpressionType::kInteger));
    EXPECT_EQ(expr->as<IntegerLiteral>().value(), 42);

    // 测试字符串字面量
    expr = ParseExpression("\"hello\"");
    ASSERT_TRUE(expr->is(ExpressionType::kString));
    EXPECT_EQ(expr->as<StringLiteral>().value(), "hello");

    // 测试布尔字面量
    expr = ParseExpression("true");
    ASSERT_TRUE(expr->is(ExpressionType::kBoolean));
    EXPECT_TRUE(expr->as<BooleanLiteral>().value());

    expr = ParseExpression("false");
    ASSERT_TRUE(expr->is(ExpressionType::kBoolean));
    EXPECT_FALSE(expr->as<BooleanLiteral>().value());

    // 测试null字面量
    expr = ParseExpression("null");
    ASSERT_TRUE(expr->is(ExpressionType::kNull));

    // 测试undefined字面量
    expr = ParseExpression("undefined");
    ASSERT_TRUE(expr->is(ExpressionType::kUndefined));
}

// 测试解析标识符
TEST_F(ParserTest, ParseIdentifier) {
    auto expr = ParseExpression("foo");
    ASSERT_TRUE(expr->is(ExpressionType::kIdentifier));
    EXPECT_EQ(expr->as<Identifier>().name(), "foo");
}

// 测试解析二元表达式
TEST_F(ParserTest, ParseBinaryExpression) {
    // 测试加法
    auto expr = ParseExpression("1 + 2");
    ASSERT_TRUE(expr->is(ExpressionType::kBinaryExpression));
    auto& binary = expr->as<BinaryExpression>();
    EXPECT_EQ(binary.op(), TokenType::kOpAdd);
    ASSERT_TRUE(binary.left()->is(ExpressionType::kInteger));
    EXPECT_EQ(binary.left()->as<IntegerLiteral>().value(), 1);
    ASSERT_TRUE(binary.right()->is(ExpressionType::kInteger));
    EXPECT_EQ(binary.right()->as<IntegerLiteral>().value(), 2);

    // 测试乘法
    expr = ParseExpression("3 * 4");
    ASSERT_TRUE(expr->is(ExpressionType::kBinaryExpression));
    auto& mul_binary = expr->as<BinaryExpression>();
    EXPECT_EQ(mul_binary.op(), TokenType::kOpMul);
    ASSERT_TRUE(mul_binary.left()->is(ExpressionType::kInteger));
    EXPECT_EQ(mul_binary.left()->as<IntegerLiteral>().value(), 3);
    ASSERT_TRUE(mul_binary.right()->is(ExpressionType::kInteger));
    EXPECT_EQ(mul_binary.right()->as<IntegerLiteral>().value(), 4);

    // 测试复杂表达式
    expr = ParseExpression("1 + 2 * 3");
    ASSERT_TRUE(expr->is(ExpressionType::kBinaryExpression));
    auto& complex_binary = expr->as<BinaryExpression>();
    EXPECT_EQ(complex_binary.op(), TokenType::kOpAdd);
    ASSERT_TRUE(complex_binary.left()->is(ExpressionType::kInteger));
    EXPECT_EQ(complex_binary.left()->as<IntegerLiteral>().value(), 1);
    ASSERT_TRUE(complex_binary.right()->is(ExpressionType::kBinaryExpression));
    auto& right_binary = complex_binary.right()->as<BinaryExpression>();
    EXPECT_EQ(right_binary.op(), TokenType::kOpMul);
}

// 测试解析赋值表达式
TEST_F(ParserTest, ParseAssignmentExpression) {
    auto expr = ParseExpression("x = 42");
    ASSERT_TRUE(expr->is(ExpressionType::kAssignmentExpression));
    auto& assign = expr->as<AssignmentExpression>();
    EXPECT_EQ(assign.op(), TokenType::kOpAssign);
    ASSERT_TRUE(assign.left()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(assign.left()->as<Identifier>().name(), "x");
    ASSERT_TRUE(assign.right()->is(ExpressionType::kInteger));
    EXPECT_EQ(assign.right()->as<IntegerLiteral>().value(), 42);
}

// 测试解析对象表达式
TEST_F(ParserTest, ParseObjectExpression) {
    auto expr = ParseExpression("{ x: 1, y: 2 }");
    ASSERT_TRUE(expr->is(ExpressionType::kObjectExpression));
    auto& obj = expr->as<ObjectExpression>();
    ASSERT_EQ(obj.properties().size(), 2);
    
    // 检查第一个属性
    EXPECT_EQ(obj.properties()[0].key, "x");
    ASSERT_TRUE(obj.properties()[0].value->is(ExpressionType::kInteger));
    EXPECT_EQ(obj.properties()[0].value->as<IntegerLiteral>().value(), 1);
    
    // 检查第二个属性
    EXPECT_EQ(obj.properties()[1].key, "y");
    ASSERT_TRUE(obj.properties()[1].value->is(ExpressionType::kInteger));
    EXPECT_EQ(obj.properties()[1].value->as<IntegerLiteral>().value(), 2);
}

// 测试解析数组表达式
TEST_F(ParserTest, ParseArrayExpression) {
    auto expr = ParseExpression("[1, 2, 3]");
    ASSERT_TRUE(expr->is(ExpressionType::kArrayExpression));
    auto& arr = expr->as<ArrayExpression>();
    ASSERT_EQ(arr.elements().size(), 3);
    
    // 检查元素
    ASSERT_TRUE(arr.elements()[0]->is(ExpressionType::kInteger));
    EXPECT_EQ(arr.elements()[0]->as<IntegerLiteral>().value(), 1);
    
    ASSERT_TRUE(arr.elements()[1]->is(ExpressionType::kInteger));
    EXPECT_EQ(arr.elements()[1]->as<IntegerLiteral>().value(), 2);
    
    ASSERT_TRUE(arr.elements()[2]->is(ExpressionType::kInteger));
    EXPECT_EQ(arr.elements()[2]->as<IntegerLiteral>().value(), 3);
}

// 测试解析函数表达式
TEST_F(ParserTest, ParseFunctionExpression) {
    auto expr = ParseExpression("function foo(a, b) { return a + b; }");
    ASSERT_TRUE(expr->is(ExpressionType::kFunctionExpression));
    auto& func = expr->as<FunctionExpression>();
    EXPECT_EQ(func.id(), "foo");
    ASSERT_EQ(func.params().size(), 2);
    EXPECT_EQ(func.params()[0], "a");
    EXPECT_EQ(func.params()[1], "b");
    
    // 检查函数体
    ASSERT_TRUE(func.body()->is(StatementType::kBlock));
    auto& body = func.body()->as<BlockStatement>();
    ASSERT_EQ(body.statements().size(), 1);
    ASSERT_TRUE(body.statements()[0]->is(StatementType::kReturn));
}

// 测试解析箭头函数表达式
TEST_F(ParserTest, ParseArrowFunctionExpression) {
    auto expr = ParseExpression("(a, b) => a + b");
    ASSERT_TRUE(expr->is(ExpressionType::kArrowFunctionExpression));
    auto& arrow = expr->as<ArrowFunctionExpression>();
    ASSERT_EQ(arrow.params().size(), 2);
    EXPECT_EQ(arrow.params()[0], "a");
    EXPECT_EQ(arrow.params()[1], "b");
    
    // 检查函数体
    ASSERT_TRUE(arrow.body()->is(StatementType::kExpression));
}

// 测试解析变量声明
TEST_F(ParserTest, ParseVariableDeclaration) {
    auto stmt = ParseStatement("let x = 42;");
    ASSERT_TRUE(stmt->is(StatementType::kVariableDeclaration));
    auto& var_decl = stmt->as<VariableDeclaration>();
    EXPECT_EQ(var_decl.name(), "x");
    EXPECT_EQ(var_decl.kind(), TokenType::kKwLet);
    ASSERT_TRUE(var_decl.init()->is(ExpressionType::kInteger));
    EXPECT_EQ(var_decl.init()->as<IntegerLiteral>().value(), 42);
}

// 测试解析if语句
TEST_F(ParserTest, ParseIfStatement) {
    auto stmt = ParseStatement("if (x > 0) { y = 1; }");
    ASSERT_TRUE(stmt->is(StatementType::kIf));
    auto& if_stmt = stmt->as<IfStatement>();
    
    // 检查条件
    ASSERT_TRUE(if_stmt.test()->is(ExpressionType::kBinaryExpression));
    
    // 检查consequent
    ASSERT_TRUE(if_stmt.consequent()->is(StatementType::kBlock));
    auto& consequent = if_stmt.consequent()->as<BlockStatement>();
    ASSERT_EQ(consequent.statements().size(), 1);
    
    // 检查alternate（应该为空）
    EXPECT_EQ(if_stmt.alternate(), nullptr);
}

// 测试解析for循环
TEST_F(ParserTest, ParseForStatement) {
    auto stmt = ParseStatement("for (let i = 0; i < 10; i++) { sum += i; }");
    ASSERT_TRUE(stmt->is(StatementType::kFor));
    auto& for_stmt = stmt->as<ForStatement>();
    
    // 检查初始化语句
    ASSERT_TRUE(for_stmt.init()->is(StatementType::kVariableDeclaration));
    
    // 检查条件表达式
    ASSERT_TRUE(for_stmt.test()->is(ExpressionType::kBinaryExpression));
    
    // 检查更新表达式
    ASSERT_TRUE(for_stmt.update()->is(ExpressionType::kUnaryExpression));
    
    // 检查循环体
    ASSERT_TRUE(for_stmt.body()->is(StatementType::kBlock));
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
    ASSERT_TRUE(parser->statements()[0]->is(StatementType::kVariableDeclaration));
    
    // 检查第四个语句是函数表达式
    ASSERT_TRUE(parser->statements()[3]->is(StatementType::kExpression));
    auto& func_stmt = parser->statements()[3]->as<ExpressionStatement>();
    ASSERT_TRUE(func_stmt.expression()->is(ExpressionType::kFunctionExpression));
}

// 测试解析一元表达式
TEST_F(ParserTest, ParseUnaryExpression) {
    // 测试前缀一元运算符
    auto expr = ParseExpression("-42");
    ASSERT_TRUE(expr->is(ExpressionType::kUnaryExpression));
    auto& unary = expr->as<UnaryExpression>();
    EXPECT_EQ(unary.op(), TokenType::kOpSub);
    EXPECT_TRUE(unary.is_prefix());
    ASSERT_TRUE(unary.argument()->is(ExpressionType::kInteger));
    EXPECT_EQ(unary.argument()->as<IntegerLiteral>().value(), 42);
    
    // 测试前缀自增
    expr = ParseExpression("++x");
    ASSERT_TRUE(expr->is(ExpressionType::kUnaryExpression));
    auto& prefix_inc = expr->as<UnaryExpression>();
    EXPECT_EQ(prefix_inc.op(), TokenType::kOpPrefixInc);
    EXPECT_TRUE(prefix_inc.is_prefix());
    ASSERT_TRUE(prefix_inc.argument()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(prefix_inc.argument()->as<Identifier>().name(), "x");
    
    // 测试后缀自增
    expr = ParseExpression("x++");
    ASSERT_TRUE(expr->is(ExpressionType::kUnaryExpression));
    auto& postfix_inc = expr->as<UnaryExpression>();
    EXPECT_EQ(postfix_inc.op(), TokenType::kOpSuffixInc);
    EXPECT_FALSE(postfix_inc.is_prefix());
    ASSERT_TRUE(postfix_inc.argument()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(postfix_inc.argument()->as<Identifier>().name(), "x");
    
    // 测试逻辑非
    expr = ParseExpression("!true");
    ASSERT_TRUE(expr->is(ExpressionType::kUnaryExpression));
    auto& logical_not = expr->as<UnaryExpression>();
    EXPECT_EQ(logical_not.op(), TokenType::kOpNot);
    EXPECT_TRUE(logical_not.is_prefix());
    ASSERT_TRUE(logical_not.argument()->is(ExpressionType::kBoolean));
    EXPECT_TRUE(logical_not.argument()->as<BooleanLiteral>().value());
    
    // 测试typeof运算符
    expr = ParseExpression("typeof x");
    ASSERT_TRUE(expr->is(ExpressionType::kUnaryExpression));
    auto& typeof_op = expr->as<UnaryExpression>();
    EXPECT_EQ(typeof_op.op(), TokenType::kKwTypeof);
    EXPECT_TRUE(typeof_op.is_prefix());
    ASSERT_TRUE(typeof_op.argument()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(typeof_op.argument()->as<Identifier>().name(), "x");
}

// 测试解析成员表达式和调用表达式
TEST_F(ParserTest, ParseMemberAndCallExpression) {
    // 测试点访问成员表达式
    auto expr = ParseExpression("obj.prop");
    ASSERT_TRUE(expr->is(ExpressionType::kMemberExpression));
    auto& member = expr->as<MemberExpression>();
    ASSERT_TRUE(member.object()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(member.object()->as<Identifier>().name(), "obj");
    ASSERT_TRUE(member.property()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(member.property()->as<Identifier>().name(), "prop");
    EXPECT_FALSE(member.computed());
    EXPECT_FALSE(member.optional());
    
    // 测试计算属性成员表达式
    expr = ParseExpression("arr[0]");
    ASSERT_TRUE(expr->is(ExpressionType::kMemberExpression));
    auto& computed_member = expr->as<MemberExpression>();
    ASSERT_TRUE(computed_member.object()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(computed_member.object()->as<Identifier>().name(), "arr");
    ASSERT_TRUE(computed_member.property()->is(ExpressionType::kInteger));
    EXPECT_EQ(computed_member.property()->as<IntegerLiteral>().value(), 0);
    EXPECT_TRUE(computed_member.computed());
    EXPECT_FALSE(computed_member.optional());
    
    // 测试可选链成员表达式
    expr = ParseExpression("obj?.prop");
    ASSERT_TRUE(expr->is(ExpressionType::kMemberExpression));
    auto& optional_member = expr->as<MemberExpression>();
    ASSERT_TRUE(optional_member.object()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(optional_member.object()->as<Identifier>().name(), "obj");
    ASSERT_TRUE(optional_member.property()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(optional_member.property()->as<Identifier>().name(), "prop");
    EXPECT_FALSE(optional_member.computed());
    EXPECT_TRUE(optional_member.optional());
    
    // 测试函数调用表达式
    expr = ParseExpression("func(1, 2)");
    ASSERT_TRUE(expr->is(ExpressionType::kCallExpression));
    auto& call = expr->as<CallExpression>();
    ASSERT_TRUE(call.callee()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(call.callee()->as<Identifier>().name(), "func");
    ASSERT_EQ(call.arguments().size(), 2);
    ASSERT_TRUE(call.arguments()[0]->is(ExpressionType::kInteger));
    EXPECT_EQ(call.arguments()[0]->as<IntegerLiteral>().value(), 1);
    ASSERT_TRUE(call.arguments()[1]->is(ExpressionType::kInteger));
    EXPECT_EQ(call.arguments()[1]->as<IntegerLiteral>().value(), 2);
    
    // 测试链式调用
    expr = ParseExpression("obj.method().prop");
    ASSERT_TRUE(expr->is(ExpressionType::kMemberExpression));
    auto& chained = expr->as<MemberExpression>();
    ASSERT_TRUE(chained.object()->is(ExpressionType::kCallExpression));
    auto& method_call = chained.object()->as<CallExpression>();
    ASSERT_TRUE(method_call.callee()->is(ExpressionType::kMemberExpression));
    EXPECT_EQ(method_call.callee()->as<MemberExpression>().property()->as<Identifier>().name(), "method");
}

// 测试解析条件表达式
TEST_F(ParserTest, ParseConditionalExpression) {
    auto expr = ParseExpression("x > 0 ? 'positive' : 'negative'");
    ASSERT_TRUE(expr->is(ExpressionType::kConditionalExpression));
    auto& cond = expr->as<ConditionalExpression>();
    
    // 检查条件部分
    ASSERT_TRUE(cond.test()->is(ExpressionType::kBinaryExpression));
    auto& test = cond.test()->as<BinaryExpression>();
    EXPECT_EQ(test.op(), TokenType::kOpGt);
    
    // 检查consequent
    ASSERT_TRUE(cond.consequent()->is(ExpressionType::kString));
    EXPECT_EQ(cond.consequent()->as<StringLiteral>().value(), "positive");
    
    // 检查alternate
    ASSERT_TRUE(cond.alternate()->is(ExpressionType::kString));
    EXPECT_EQ(cond.alternate()->as<StringLiteral>().value(), "negative");
}

// 测试解析while循环语句
TEST_F(ParserTest, ParseWhileStatement) {
    auto stmt = ParseStatement("while (i < 10) { i++; }");
    ASSERT_TRUE(stmt->is(StatementType::kWhile));
    auto& while_stmt = stmt->as<WhileStatement>();
    
    // 检查条件
    ASSERT_TRUE(while_stmt.test()->is(ExpressionType::kBinaryExpression));
    auto& test = while_stmt.test()->as<BinaryExpression>();
    EXPECT_EQ(test.op(), TokenType::kOpLt);
    
    // 检查循环体
    ASSERT_TRUE(while_stmt.body()->is(StatementType::kBlock));
    auto& body = while_stmt.body()->as<BlockStatement>();
    ASSERT_EQ(body.statements().size(), 1);
    ASSERT_TRUE(body.statements()[0]->is(StatementType::kExpression));
}

// 测试解析带else的if语句
TEST_F(ParserTest, ParseIfElseStatement) {
    auto stmt = ParseStatement("if (x > 0) { y = 1; } else { y = -1; }");
    ASSERT_TRUE(stmt->is(StatementType::kIf));
    auto& if_stmt = stmt->as<IfStatement>();
    
    // 检查条件
    ASSERT_TRUE(if_stmt.test()->is(ExpressionType::kBinaryExpression));
    
    // 检查consequent
    ASSERT_TRUE(if_stmt.consequent()->is(StatementType::kBlock));
    
    // 检查alternate（不应为空）
    ASSERT_NE(if_stmt.alternate(), nullptr);
    ASSERT_TRUE(if_stmt.alternate()->is(StatementType::kBlock));
}

// 测试解析try-catch-finally语句
TEST_F(ParserTest, ParseTryCatchStatement) {
    auto stmt = ParseStatement("try { riskyOperation(); } catch (error) { handleError(error); } finally { cleanup(); }");
    ASSERT_TRUE(stmt->is(StatementType::kTry));
    auto& try_stmt = stmt->as<TryStatement>();
    
    // 检查try块
    ASSERT_TRUE(try_stmt.block()->is(StatementType::kBlock));
    
    // 检查catch子句
    ASSERT_NE(try_stmt.handler(), nullptr);
    ASSERT_TRUE(try_stmt.handler()->is(StatementType::kCatch));
    auto& catch_clause = try_stmt.handler()->as<CatchClause>();
    EXPECT_EQ(catch_clause.param()->name(), "error");
    
    // 检查finally子句
    ASSERT_NE(try_stmt.finalizer(), nullptr);
    ASSERT_TRUE(try_stmt.finalizer()->is(StatementType::kFinally));
}

// 测试解析throw语句
TEST_F(ParserTest, ParseThrowStatement) {
    auto stmt = ParseStatement("throw new Error('Something went wrong');");
    ASSERT_TRUE(stmt->is(StatementType::kThrow));
    auto& throw_stmt = stmt->as<ThrowStatement>();
    
    // 检查抛出的表达式
    ASSERT_TRUE(throw_stmt.argument()->is(ExpressionType::kNewExpression));
}

// 测试解析break和continue语句
TEST_F(ParserTest, ParseBreakContinueStatement) {
    // 测试break语句
    auto break_stmt = ParseStatement("break;");
    ASSERT_TRUE(break_stmt->is(StatementType::kBreak));
    
    // 测试带标签的break语句
    auto labeled_break = ParseStatement("break outerLoop;");
    ASSERT_TRUE(labeled_break->is(StatementType::kBreak));
    EXPECT_EQ(labeled_break->as<BreakStatement>().label(), "outerLoop");
    
    // 测试continue语句
    auto cont_stmt = ParseStatement("continue;");
    ASSERT_TRUE(cont_stmt->is(StatementType::kContinue));
    
    // 测试带标签的continue语句
    auto labeled_cont = ParseStatement("continue outerLoop;");
    ASSERT_TRUE(labeled_cont->is(StatementType::kContinue));
    EXPECT_EQ(labeled_cont->as<ContinueStatement>().label(), "outerLoop");
}

// 测试解析标签语句
TEST_F(ParserTest, ParseLabeledStatement) {
    auto stmt = ParseStatement("outerLoop: for (let i = 0; i < 10; i++) { innerLoop: for (let j = 0; j < 10; j++) { if (j > 5) break outerLoop; } }");
    ASSERT_TRUE(stmt->is(StatementType::kLabeled));
    auto& labeled = stmt->as<LabeledStatement>();
    
    // 检查标签名称
    EXPECT_EQ(labeled.label(), "outerLoop");
    
    // 检查标签语句的主体
    ASSERT_TRUE(labeled.body()->is(StatementType::kFor));
}

// 测试解析return语句
TEST_F(ParserTest, ParseReturnStatement) {
    // 测试无值的return
    auto empty_return = ParseStatement("return;");
    ASSERT_TRUE(empty_return->is(StatementType::kReturn));
    EXPECT_EQ(empty_return->as<ReturnStatement>().argument(), nullptr);
    
    // 测试带值的return
    auto value_return = ParseStatement("return 42;");
    ASSERT_TRUE(value_return->is(StatementType::kReturn));
    ASSERT_NE(value_return->as<ReturnStatement>().argument(), nullptr);
    ASSERT_TRUE(value_return->as<ReturnStatement>().argument()->is(ExpressionType::kInteger));
    EXPECT_EQ(value_return->as<ReturnStatement>().argument()->as<IntegerLiteral>().value(), 42);
}

// 测试解析模板字符串
TEST_F(ParserTest, ParseTemplateLiteral) {
    auto expr = ParseExpression("`Hello, ${name}!`");
    ASSERT_TRUE(expr->is(ExpressionType::kTemplateLiteral));
    auto& template_literal = expr->as<TemplateLiteral>();
    
    // 检查模板字符串的部分
    //ASSERT_GE(template_literal.quasis().size(), 2);
    ASSERT_EQ(template_literal.expressions().size(), 1);
    
    // 检查表达式部分
    ASSERT_TRUE(template_literal.expressions()[0]->is(ExpressionType::kIdentifier));
    EXPECT_EQ(template_literal.expressions()[0]->as<Identifier>().name(), "name");
}

// 测试解析导入语句
TEST_F(ParserTest, ParseImportStatement) {
    auto stmt = ParseStatement("import { foo, bar as baz } from 'module';");
    ASSERT_TRUE(stmt->is(StatementType::kImport));
    auto& import_stmt = stmt->as<ImportDeclaration>();
    
    // 检查导入的模块
    EXPECT_EQ(import_stmt.source(), "module");
    
    // 检查导入的标识符
    //ASSERT_GE(import_stmt.specifiers().size(), 2);
}

// 测试解析导出语句
TEST_F(ParserTest, ParseExportStatement) {
    auto stmt = ParseStatement("export const PI = 3.14;");
    ASSERT_TRUE(stmt->is(StatementType::kExport));
    auto& export_stmt = stmt->as<ExportDeclaration>();
    
    // 检查导出的声明
    ASSERT_NE(export_stmt.declaration(), nullptr);
    ASSERT_TRUE(export_stmt.declaration()->is(StatementType::kVariableDeclaration));
    auto& var_decl = export_stmt.declaration()->as<VariableDeclaration>();
    EXPECT_EQ(var_decl.name(), "PI");
    EXPECT_EQ(var_decl.kind(), TokenType::kKwConst);
}

// 测试解析new表达式
TEST_F(ParserTest, ParseNewExpression) {
    auto expr = ParseExpression("new Date()");
    ASSERT_TRUE(expr->is(ExpressionType::kNewExpression));
    auto& new_expr = expr->as<NewExpression>();
    
    // 检查构造函数
    ASSERT_TRUE(new_expr.callee()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(new_expr.callee()->as<Identifier>().name(), "Date");
    
    // 检查参数（应为空）
    EXPECT_EQ(new_expr.arguments().size(), 0);
    
    // 测试带参数的new表达式
    expr = ParseExpression("new Person('John', 30)");
    ASSERT_TRUE(expr->is(ExpressionType::kNewExpression));
    auto& new_expr_with_args = expr->as<NewExpression>();
    
    // 检查构造函数
    ASSERT_TRUE(new_expr_with_args.callee()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(new_expr_with_args.callee()->as<Identifier>().name(), "Person");
    
    // 检查参数
    ASSERT_EQ(new_expr_with_args.arguments().size(), 2);
    ASSERT_TRUE(new_expr_with_args.arguments()[0]->is(ExpressionType::kString));
    ASSERT_TRUE(new_expr_with_args.arguments()[1]->is(ExpressionType::kInteger));
}

// 测试解析yield表达式
TEST_F(ParserTest, ParseYieldExpression) {
    auto expr = ParseExpression("yield value");
    ASSERT_TRUE(expr->is(ExpressionType::kYieldExpression));
    auto& yield_expr = expr->as<YieldExpression>();
    
    // 检查yield的值
    ASSERT_NE(yield_expr.argument(), nullptr);
    ASSERT_TRUE(yield_expr.argument()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(yield_expr.argument()->as<Identifier>().name(), "value");
    //EXPECT_FALSE(yield_expr.delegate());
    
    // 测试yield*表达式
    expr = ParseExpression("yield* generator()");
    ASSERT_TRUE(expr->is(ExpressionType::kYieldExpression));
    auto& yield_star_expr = expr->as<YieldExpression>();
    
    // 检查yield*的值
    ASSERT_NE(yield_star_expr.argument(), nullptr);
    ASSERT_TRUE(yield_star_expr.argument()->is(ExpressionType::kCallExpression));
    //EXPECT_TRUE(yield_star_expr.delegate());
}

// 测试解析await表达式
TEST_F(ParserTest, ParseAwaitExpression) {
    auto expr = ParseExpression("await promise");
    ASSERT_TRUE(expr->is(ExpressionType::kAwaitExpression));
    auto& await_expr = expr->as<AwaitExpression>();
    
    // 检查await的值
    ASSERT_NE(await_expr.argument(), nullptr);
    ASSERT_TRUE(await_expr.argument()->is(ExpressionType::kIdentifier));
    EXPECT_EQ(await_expr.argument()->as<Identifier>().name(), "promise");
}

// 测试解析类表达式
TEST_F(ParserTest, ParseClassExpression) {
    auto expr = ParseExpression("class Person { constructor(name) { this.name = name; } getName() { return this.name; } }");
    ASSERT_TRUE(expr->is(ExpressionType::kClassExpression));
    //auto& class_expr = expr->as<ClassExpression>();
    //
    //// 检查类名
    //EXPECT_EQ(class_expr.id(), "Person");
    //
    //// 检查类体
    //ASSERT_GE(class_expr.body().size(), 2);  // 至少有constructor和getName方法
}

// 测试解析import表达式
TEST_F(ParserTest, ParseImportExpression) {
    auto expr = ParseExpression("import('module')");
    ASSERT_TRUE(expr->is(ExpressionType::kImportExpression));
    auto& import_expr = expr->as<ImportExpression>();
    
    // 检查导入的模块
    ASSERT_TRUE(import_expr.source()->is(ExpressionType::kString));
    EXPECT_EQ(import_expr.source()->as<StringLiteral>().value(), "module");
}

// 测试解析复杂的嵌套表达式
TEST_F(ParserTest, ParseComplexNestedExpression) {
    auto expr = ParseExpression("(a + b) * (c - d) / Math.sqrt(e ** 2 + f ** 2)");
    ASSERT_TRUE(expr->is(ExpressionType::kBinaryExpression));
    
    // 检查顶层操作符（除法）
    auto& div_expr = expr->as<BinaryExpression>();
    EXPECT_EQ(div_expr.op(), TokenType::kOpDiv);
    
    // 检查左侧（乘法表达式）
    ASSERT_TRUE(div_expr.left()->is(ExpressionType::kBinaryExpression));
    auto& mul_expr = div_expr.left()->as<BinaryExpression>();
    EXPECT_EQ(mul_expr.op(), TokenType::kOpMul);
    
    // 检查右侧（函数调用）
    ASSERT_TRUE(div_expr.right()->is(ExpressionType::kCallExpression));
    auto& call_expr = div_expr.right()->as<CallExpression>();
    ASSERT_TRUE(call_expr.callee()->is(ExpressionType::kMemberExpression));
}

} // namespace test
} // namespace compiler
} // namespace mjs
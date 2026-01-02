/**
 * @file code_generator_test.cpp
 * @brief CodeGenerator 单元测试
 *
 * 测试代码生成器 (CodeGenerator) 类的功能,包括:
 * - 构造函数和析构函数
 * - 添加C++函数 (AddCppFunction)
 * - 生成代码 (Generate)
 * - 生成表达式代码 (GenerateExpression)
 * - 生成语句代码 (GenerateStatement)
 * - 生成函数体代码 (GenerateFunctionBody)
 * - 生成左值存储代码 (GenerateLValueStore)
 * - 生成条件相等判断代码 (GenerateIfEq)
 * - 生成参数列表代码 (GenerateParamList)
 * - 分配常量 (AllocateConst)
 * - 获取常量值 (GetConstValueByIndex)
 * - 创建常量值 (MakeConstValue)
 * - ScopeManager 和 JumpManager 访问器
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include <mjs/value.h>
#include "src/compiler/code_generator.h"
#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/member_expression.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/boolean_literal.h"
#include "src/compiler/expression_impl/undefined_literal.h"
#include "src/compiler/expression_impl/null_literal.h"
#include "src/compiler/expression_impl/float_literal.h"
#include "src/compiler/expression_impl/template_element.h"
#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/statement_impl/expression_statement.h"
#include "src/compiler/statement_impl/return_statement.h"
#include "tests/unit/test_helpers.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class CodeGeneratorTest
 * @brief CodeGenerator 测试类
 */
class CodeGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试环境
        test_env_ = std::make_unique<mjs::test::TestEnvironment>();
        runtime_ = test_env_->runtime();
        context_ = std::make_unique<Context>(runtime_);
        module_def_ = test_env_->module_def();
        function_def_ = test_env_->function_def();
    }

    void TearDown() override {
        context_.reset();
        test_env_.reset();
    }

    /**
     * @brief 创建解析器
     */
    std::unique_ptr<Parser> CreateParser(const std::string& source) {
        auto lexer = std::make_unique<Lexer>(source);
        auto parser = std::make_unique<Parser>(lexer.get());
        parser->ParseProgram();
        return parser;
    }

    std::unique_ptr<mjs::test::TestEnvironment> test_env_;
    Runtime* runtime_;
    std::unique_ptr<Context> context_;
    ModuleDef* module_def_;
    FunctionDef* function_def_;
};

// ============================================================================
// 构造函数和析构函数测试
// ============================================================================

/**
 * @test 测试CodeGenerator构造函数
 */
TEST_F(CodeGeneratorTest, Constructor_ValidContextAndParser) {
    auto parser = CreateParser("let x;");

    EXPECT_NO_THROW({
        CodeGenerator generator(context_.get(), parser.get());
    });
}

/**
 * @test 测试CodeGenerator不可复制
 */
TEST_F(CodeGeneratorTest, CodeGenerator_IsNonCopyable) {
    auto parser = CreateParser("let x;");
    CodeGenerator gen1(context_.get(), parser.get());

    // CodeGenerator 继承自 noncopyable，不应该能复制
    // CodeGenerator gen2 = gen1;  // 编译错误
}

// ============================================================================
// AddCppFunction 测试
// ============================================================================

/**
 * @test 测试添加C++函数
 */
TEST_F(CodeGeneratorTest, AddCppFunction_Basic) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 需要先进入作用域
    generator.scope_manager().EnterScope(function_def_);

    // 创建一个简单的C++函数
    CppFunction cpp_func = [](Context* ctx, uint32_t par_count, const StackFrame& stack) -> Value {
        return Value(42);
    };

    EXPECT_NO_THROW({
        generator.AddCppFunction(function_def_, "testFunc", cpp_func);
    });

    generator.scope_manager().ExitScope();
}

/**
 * @test 测试添加多个C++函数
 */
TEST_F(CodeGeneratorTest, AddCppFunction_MultipleFunctions) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 需要先进入作用域
    generator.scope_manager().EnterScope(function_def_);

    CppFunction func1 = [](Context* ctx, uint32_t par_count, const StackFrame& stack) -> Value {
        return Value(1);
    };

    CppFunction func2 = [](Context* ctx, uint32_t par_count, const StackFrame& stack) -> Value {
        return Value(2);
    };

    EXPECT_NO_THROW({
        generator.AddCppFunction(function_def_, "func1", func1);
        generator.AddCppFunction(function_def_, "func2", func2);
    });

    generator.scope_manager().ExitScope();
}

// ============================================================================
// Generate 测试
// ============================================================================

/**
 * @test 测试生成空模块
 */
TEST_F(CodeGeneratorTest, Generate_EmptyModule) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    Value result = generator.Generate("test_module", "");

    EXPECT_FALSE(result.IsUndefined());
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @test 测试生成简单语句模块
 */
TEST_F(CodeGeneratorTest, Generate_SimpleStatements) {
    auto parser = CreateParser("let x; let y;");
    CodeGenerator generator(context_.get(), parser.get());

    Value result = generator.Generate("test_module", "let x; let y;");

    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @test 测试生成包含import的模块
 */
TEST_F(CodeGeneratorTest, Generate_WithImports) {
    auto parser = CreateParser("import { foo } from 'module';");
    CodeGenerator generator(context_.get(), parser.get());

    Value result = generator.Generate("test_module", "import { foo } from 'module';");

    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @test 测试生成混合语句和import的模块
 */
TEST_F(CodeGeneratorTest, Generate_MixedImportsAndStatements) {
    auto parser = CreateParser("import { foo } from 'module'; let x;");
    CodeGenerator generator(context_.get(), parser.get());

    Value result = generator.Generate("test_module",
                                      "import { foo } from 'module'; let x;");

    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @test 测试生成复杂代码模块
 */
TEST_F(CodeGeneratorTest, Generate_ComplexCode) {
    std::string source = R"(
        let x = 10;
        const y = 20;
        function add(a, b) {
            return a + b;
        }
        class MyClass {
            constructor() {
                this.value = 0;
            }
        }
    )";

    auto parser = CreateParser(source);
    CodeGenerator generator(context_.get(), parser.get());

    Value result = generator.Generate("test_module", source);

    EXPECT_TRUE(result.IsModuleDef());
}

// ============================================================================
// GenerateExpression 测试
// ============================================================================

/**
 * @test 测试生成标识符表达式代码
 */
TEST_F(CodeGeneratorTest, GenerateExpression_Identifier) {
    auto parser = CreateParser("x;");  // 添加分号使其成为完整语句
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<Identifier>(0, 0, "x");
    EXPECT_NO_THROW({
        generator.GenerateExpression(function_def_, expr.get());
    });
}

/**
 * @test 测试生成整数字面量表达式代码
 */
TEST_F(CodeGeneratorTest, GenerateExpression_IntegerLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    EXPECT_NO_THROW({
        generator.GenerateExpression(function_def_, expr.get());
    });
}

/**
 * @test 测试生成字符串字面量表达式代码
 */
TEST_F(CodeGeneratorTest, GenerateExpression_StringLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<StringLiteral>(0, 0, "hello");
    EXPECT_NO_THROW({
        generator.GenerateExpression(function_def_, expr.get());
    });
}

/**
 * @test 测试生成布尔字面量表达式代码
 */
TEST_F(CodeGeneratorTest, GenerateExpression_BooleanLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr1 = std::make_unique<BooleanLiteral>(0, 0, true);
    auto expr2 = std::make_unique<BooleanLiteral>(0, 0, false);

    EXPECT_NO_THROW({
        generator.GenerateExpression(function_def_, expr1.get());
        generator.GenerateExpression(function_def_, expr2.get());
    });
}

// ============================================================================
// GenerateStatement 测试
// ============================================================================

/**
 * @test 测试生成表达式语句代码
 */
TEST_F(CodeGeneratorTest, GenerateStatement_ExpressionStatement) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    auto stmt = std::make_unique<ExpressionStatement>(0, 0, std::move(expr));

    EXPECT_NO_THROW({
        generator.GenerateStatement(function_def_, stmt.get());
    });
}

/**
 * @test 测试生成块语句代码
 */
TEST_F(CodeGeneratorTest, GenerateStatement_BlockStatement) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Statement>> statements;
    auto stmt = std::make_unique<BlockStatement>(0, 0, std::move(statements));

    EXPECT_NO_THROW({
        generator.GenerateStatement(function_def_, stmt.get());
    });
}

/**
 * @test 测试生成嵌套块语句代码
 */
TEST_F(CodeGeneratorTest, GenerateStatement_NestedBlockStatement) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 创建内层块
    std::vector<std::unique_ptr<Statement>> inner_statements;
    auto inner_expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    inner_statements.push_back(std::make_unique<ExpressionStatement>(0, 0, std::move(inner_expr)));
    auto inner_block = std::make_unique<BlockStatement>(0, 0, std::move(inner_statements));

    // 创建外层块
    std::vector<std::unique_ptr<Statement>> outer_statements;
    outer_statements.push_back(std::move(inner_block));
    auto outer_block = std::make_unique<BlockStatement>(0, 0, std::move(outer_statements));

    EXPECT_NO_THROW({
        generator.GenerateStatement(function_def_, outer_block.get());
    });
}

// ============================================================================
// GenerateFunctionBody 测试
// ============================================================================

/**
 * @test 测试生成块语句函数体
 */
TEST_F(CodeGeneratorTest, GenerateFunctionBody_BlockStatement) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Statement>> statements;
    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    statements.push_back(std::make_unique<ExpressionStatement>(0, 0, std::move(expr)));
    auto block = std::make_unique<BlockStatement>(0, 0, std::move(statements));

    EXPECT_NO_THROW({
        generator.GenerateFunctionBody(function_def_, block.get());
    });
}

/**
 * @test 测试生成带return的函数体
 */
TEST_F(CodeGeneratorTest, GenerateFunctionBody_WithReturn) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Statement>> statements;
    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    statements.push_back(std::make_unique<ReturnStatement>(0, 0, std::move(expr)));
    auto block = std::make_unique<BlockStatement>(0, 0, std::move(statements));

    EXPECT_NO_THROW({
        generator.GenerateFunctionBody(function_def_, block.get());
    });
}

/**
 * @test 测试生成不带return的函数体(自动补全)
 */
TEST_F(CodeGeneratorTest, GenerateFunctionBody_WithoutReturn) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Statement>> statements;
    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    statements.push_back(std::make_unique<ExpressionStatement>(0, 0, std::move(expr)));
    auto block = std::make_unique<BlockStatement>(0, 0, std::move(statements));

    EXPECT_NO_THROW({
        generator.GenerateFunctionBody(function_def_, block.get());
        // 应该自动补全 undefined 和 return
    });
}

/**
 * @test 测试生成多语句函数体
 */
TEST_F(CodeGeneratorTest, GenerateFunctionBody_MultipleStatements) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Statement>> statements;
    statements.push_back(std::make_unique<ExpressionStatement>(0, 0,
        std::make_unique<IntegerLiteral>(0, 0, 1)));
    statements.push_back(std::make_unique<ExpressionStatement>(0, 0,
        std::make_unique<IntegerLiteral>(0, 0, 2)));
    statements.push_back(std::make_unique<ExpressionStatement>(0, 0,
        std::make_unique<IntegerLiteral>(0, 0, 3)));

    auto block = std::make_unique<BlockStatement>(0, 0, std::move(statements));

    EXPECT_NO_THROW({
        generator.GenerateFunctionBody(function_def_, block.get());
    });
}

/**
 * @test 测试生成表达式函数体
 */
TEST_F(CodeGeneratorTest, GenerateFunctionBody_ExpressionBody) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    auto expr_stmt = std::make_unique<ExpressionStatement>(0, 0, std::move(expr));

    EXPECT_NO_THROW({
        generator.GenerateFunctionBody(function_def_, expr_stmt.get());
    });
}

// ============================================================================
// GenerateLValueStore 测试
// ============================================================================

/**
 * @test 测试生成标识符左值存储代码
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_Identifier) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 首先分配变量
    generator.scope_manager().EnterScope(function_def_);
    generator.scope_manager().AllocateVar("x", VarFlags::kNone);

    auto ident = std::make_unique<Identifier>(0, 0, "x");

    EXPECT_NO_THROW({
        generator.GenerateLValueStore(function_def_, ident.get());
    });

    generator.scope_manager().ExitScope();
}

/**
 * @test 测试生成const标识符左值存储(应该抛出异常)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_ConstIdentifier) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 分配const变量
    generator.scope_manager().EnterScope(function_def_);
    generator.scope_manager().AllocateVar("x", VarFlags::kConst);

    auto ident = std::make_unique<Identifier>(0, 0, "x");

    EXPECT_THROW({
        generator.GenerateLValueStore(function_def_, ident.get());
    }, SyntaxError);

    generator.scope_manager().ExitScope();
}

/**
 * @test 测试生成未定义变量左值存储(应该抛出异常)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_UndefinedIdentifier) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    generator.scope_manager().EnterScope(function_def_);

    auto ident = std::make_unique<Identifier>(0, 0, "undefinedVar");

    EXPECT_THROW({
        generator.GenerateLValueStore(function_def_, ident.get());
    }, SyntaxError);

    generator.scope_manager().ExitScope();
}

/**
 * @test 测试生成成员表达式左值存储代码(计算属性)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_MemberExpression_Computed) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto obj = std::make_unique<Identifier>(0, 0, "obj");
    auto prop = std::make_unique<StringLiteral>(0, 0, "prop");
    auto member = std::make_unique<MemberExpression>(0, 0, std::move(obj), std::move(prop), false, true, false);

    EXPECT_NO_THROW({
        generator.GenerateLValueStore(function_def_, member.get());
    });
}

/**
 * @test 测试生成成员表达式左值存储代码(普通属性)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_MemberExpression_NonComputed) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto obj = std::make_unique<Identifier>(0, 0, "obj");
    auto prop = std::make_unique<Identifier>(0, 0, "x");
    auto member = std::make_unique<MemberExpression>(0, 0, std::move(obj), std::move(prop), false, false, false);

    EXPECT_NO_THROW({
        generator.GenerateLValueStore(function_def_, member.get());
    });
}

/**
 * @test 测试生成非左值表达式存储代码(应该抛出异常)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_NonLValueExpression) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto int_lit = std::make_unique<IntegerLiteral>(0, 0, 42);

    EXPECT_THROW({
        generator.GenerateLValueStore(function_def_, int_lit.get());
    }, std::runtime_error);
}

/**
 * @test 测试生成不支持的左值表达式类型(应该抛出异常)
 */
TEST_F(CodeGeneratorTest, GenerateLValueStore_UnsupportedExpressionType) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 创建一个不支持的表达式类型作为左值
    // 这里可以扩展测试其他不支持的表达式类型
    auto bool_lit = std::make_unique<BooleanLiteral>(0, 0, true);

    EXPECT_THROW({
        generator.GenerateLValueStore(function_def_, bool_lit.get());
    }, std::runtime_error);
}

// ============================================================================
// GenerateIfEq 测试
// ============================================================================

/**
 * @test 测试生成IfEq代码
 */
TEST_F(CodeGeneratorTest, GenerateIfEq_Basic) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        generator.GenerateIfEq(function_def_);
    });
}

/**
 * @test 测试生成多个IfEq代码
 */
TEST_F(CodeGeneratorTest, GenerateIfEq_Multiple) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        generator.GenerateIfEq(function_def_);
        generator.GenerateIfEq(function_def_);
        generator.GenerateIfEq(function_def_);
    });
}

// ============================================================================
// GenerateParamList 测试
// ============================================================================

/**
 * @test 测试生成空参数列表代码
 */
TEST_F(CodeGeneratorTest, GenerateParamList_Empty) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Expression>> params;

    EXPECT_NO_THROW({
        generator.GenerateParamList(function_def_, params);
    });
}

/**
 * @test 测试生成单个参数列表代码
 */
TEST_F(CodeGeneratorTest, GenerateParamList_SingleParameter) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Expression>> params;
    params.push_back(std::make_unique<IntegerLiteral>(0, 0, 42));

    EXPECT_NO_THROW({
        generator.GenerateParamList(function_def_, params);
    });
}

/**
 * @test 测试生成多个参数列表代码
 */
TEST_F(CodeGeneratorTest, GenerateParamList_MultipleParameters) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Expression>> params;
    params.push_back(std::make_unique<IntegerLiteral>(0, 0, 1));
    params.push_back(std::make_unique<IntegerLiteral>(0, 0, 2));
    params.push_back(std::make_unique<IntegerLiteral>(0, 0, 3));

    EXPECT_NO_THROW({
        generator.GenerateParamList(function_def_, params);
    });
}

/**
 * @test 测试生成混合类型参数列表代码
 */
TEST_F(CodeGeneratorTest, GenerateParamList_MixedTypes) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Expression>> params;
    params.push_back(std::make_unique<IntegerLiteral>(0, 0, 42));
    params.push_back(std::make_unique<StringLiteral>(0, 0, "hello"));
    params.push_back(std::make_unique<BooleanLiteral>(0, 0, true));
    params.push_back(std::make_unique<Identifier>(0, 0, "x"));

    EXPECT_NO_THROW({
        generator.GenerateParamList(function_def_, params);
    });
}

// ============================================================================
// AllocateConst 测试
// ============================================================================

/**
 * @test 测试分配整型常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_Integer) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(42));
    EXPECT_GE(idx, 0);
}

/**
 * @test 测试分配浮点型常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_Float) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(3.14));
    EXPECT_GE(idx, 0);
}

/**
 * @test 测试分配字符串常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_String) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(String::New("hello")));
    EXPECT_GE(idx, 0);
}

/**
 * @test 测试分配布尔型常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_Boolean) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx1 = generator.AllocateConst(Value(true));
    ConstIndex idx2 = generator.AllocateConst(Value(false));
    EXPECT_GE(idx1, 0);
    EXPECT_GE(idx2, 0);
}

/**
 * @test 测试分配null常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_Null) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(nullptr));
    EXPECT_GE(idx, 0);
}

/**
 * @test 测试分配undefined常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_Undefined) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value());
    EXPECT_GE(idx, 0);
}

/**
 * @test 测试分配多个常量
 */
TEST_F(CodeGeneratorTest, AllocateConst_MultipleConstants) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx1 = generator.AllocateConst(Value(1));
    ConstIndex idx2 = generator.AllocateConst(Value(2));
    ConstIndex idx3 = generator.AllocateConst(Value(3));

    EXPECT_GE(idx1, 0);
    EXPECT_GE(idx2, 0);
    EXPECT_GE(idx3, 0);
}

/**
 * @test 测试分配相同值的常量(应该返回相同索引)
 */
TEST_F(CodeGeneratorTest, AllocateConst_SameValue) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx1 = generator.AllocateConst(Value(42));
    ConstIndex idx2 = generator.AllocateConst(Value(42));

    EXPECT_EQ(idx1, idx2);
}

// ============================================================================
// GetConstValueByIndex 测试
// ============================================================================

/**
 * @test 测试通过索引获取常量值
 */
TEST_F(CodeGeneratorTest, GetConstValueByIndex_Integer) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(42));
    const Value& val = generator.GetConstValueByIndex(idx);

    EXPECT_TRUE(val.IsNumber());
    EXPECT_TRUE(val.IsInt64());
    EXPECT_EQ(val.i64(), 42);
}

/**
 * @test 测试获取字符串常量值
 */
TEST_F(CodeGeneratorTest, GetConstValueByIndex_String) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(String::New("test")));
    const Value& val = generator.GetConstValueByIndex(idx);

    EXPECT_TRUE(val.IsString());
    EXPECT_STREQ(val.string_view(), "test");
}

/**
 * @test 测试获取布尔常量值
 */
TEST_F(CodeGeneratorTest, GetConstValueByIndex_Boolean) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx1 = generator.AllocateConst(Value(true));
    ConstIndex idx2 = generator.AllocateConst(Value(false));

    const Value& val1 = generator.GetConstValueByIndex(idx1);
    const Value& val2 = generator.GetConstValueByIndex(idx2);

    EXPECT_TRUE(val1.IsBoolean());
    EXPECT_TRUE(val1.boolean());
    EXPECT_TRUE(val2.IsBoolean());
    EXPECT_FALSE(val2.boolean());
}

/**
 * @test 测试获取null常量值
 */
TEST_F(CodeGeneratorTest, GetConstValueByIndex_Null) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value(nullptr));
    const Value& val = generator.GetConstValueByIndex(idx);

    EXPECT_TRUE(val.IsNull());
}

/**
 * @test 测试获取undefined常量值
 */
TEST_F(CodeGeneratorTest, GetConstValueByIndex_Undefined) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    ConstIndex idx = generator.AllocateConst(Value());
    const Value& val = generator.GetConstValueByIndex(idx);

    EXPECT_TRUE(val.IsUndefined());
}

// ============================================================================
// MakeConstValue 测试
// ============================================================================

/**
 * @test 测试从undefined表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_UndefinedLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<UndefinedLiteral>(0, 0);
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsUndefined());
}

/**
 * @test 测试从null表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_NullLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<NullLiteral>(0, 0);
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsNull());
}

/**
 * @test 测试从布尔表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_BooleanLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr1 = std::make_unique<BooleanLiteral>(0, 0, true);
    auto expr2 = std::make_unique<BooleanLiteral>(0, 0, false);

    Value val1 = generator.MakeConstValue(function_def_, expr1.get());
    Value val2 = generator.MakeConstValue(function_def_, expr2.get());

    EXPECT_TRUE(val1.IsBoolean());
    EXPECT_TRUE(val1.boolean());
    EXPECT_TRUE(val2.IsBoolean());
    EXPECT_FALSE(val2.boolean());
}

/**
 * @test 测试从整数表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_IntegerLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsNumber());
    EXPECT_TRUE(val.IsInt64());
    EXPECT_EQ(val.i64(), 42);
}

/**
 * @test 测试从浮点数表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_FloatLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<FloatLiteral>(0, 0, 3.14);
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsNumber());
    EXPECT_DOUBLE_EQ(val.f64(), 3.14);
}

/**
 * @test 测试从字符串表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_StringLiteral) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<StringLiteral>(0, 0, "hello");
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsString());
    EXPECT_STREQ(val.string_view(), "hello");
}

/**
 * @test 测试从模板元素表达式创建常量值
 */
TEST_F(CodeGeneratorTest, MakeConstValue_TemplateElement) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    auto expr = std::make_unique<TemplateElement>(0, 0, "template_value");
    Value val = generator.MakeConstValue(function_def_, expr.get());

    EXPECT_TRUE(val.IsString());
    EXPECT_STREQ("template_value", val.string_view());
}

/**
 * @test 测试从不支持的表达式创建常量值(应该抛出异常)
 */
TEST_F(CodeGeneratorTest, MakeConstValue_UnsupportedExpression) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 创建一个不支持的表达式类型(Identifier)
    auto expr = std::make_unique<Identifier>(0, 0, "x");

    EXPECT_THROW({
        generator.MakeConstValue(function_def_, expr.get());
    }, SyntaxError);
}

// ============================================================================
// ScopeManager 访问器测试
// ============================================================================

/**
 * @test 测试获取ScopeManager引用
 */
TEST_F(CodeGeneratorTest, ScopeManagerAccessor) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        auto& sm = generator.scope_manager();
        auto& sm_const = const_cast<const CodeGenerator&>(generator).scope_manager();
        (void)sm;
        (void)sm_const;
    });
}

// ============================================================================
// JumpManager 访问器测试
// ============================================================================

/**
 * @test 测试获取JumpManager引用
 */
TEST_F(CodeGeneratorTest, JumpManagerAccessor) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        auto& jm = generator.jump_manager();
        auto& jm_const = const_cast<const CodeGenerator&>(generator).jump_manager();
        (void)jm;
        (void)jm_const;
    });
}

// ============================================================================
// 集成测试
// ============================================================================

/**
 * @test 测试完整的代码生成流程
 */
TEST_F(CodeGeneratorTest, Integration_CompleteGeneration) {
    std::string source = R"(
        let x = 10;
        const y = 20;
        if (x > 0) {
            x = x + y;
        }
        return x;
    )";

    auto parser = CreateParser(source);
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        Value result = generator.Generate("test_module", source);
        EXPECT_TRUE(result.IsModuleDef());
    });
}

/**
 * @test 测试生成函数定义代码
 */
TEST_F(CodeGeneratorTest, Integration_FunctionDefinition) {
    std::string source = R"(
        function add(a, b) {
            return a + b;
        }
    )";

    auto parser = CreateParser(source);
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        Value result = generator.Generate("test_module", source);
        EXPECT_TRUE(result.IsModuleDef());
    });
}

/**
 * @test 测试生成类定义代码
 */
TEST_F(CodeGeneratorTest, Integration_ClassDefinition) {
    std::string source = R"(
        class MyClass {
            constructor(value) {
                this.value = value;
            }
            getValue() {
                return this.value;
            }
        }
    )";

    auto parser = CreateParser(source);
    CodeGenerator generator(context_.get(), parser.get());

    EXPECT_NO_THROW({
        Value result = generator.Generate("test_module", source);
        EXPECT_TRUE(result.IsModuleDef());
    });
}

// ============================================================================
// 边界情况和错误处理测试
// ============================================================================

/**
 * @test 测试多次Generate调用(ScopeManager重置)
 */
TEST_F(CodeGeneratorTest, MultipleGenerateCalls) {
    auto parser1 = CreateParser("let x;");
    CodeGenerator generator(context_.get(), parser1.get());

    Value result1 = generator.Generate("module1", "let x;");
    EXPECT_TRUE(result1.IsModuleDef());

    auto parser2 = CreateParser("let y;");
    Value result2 = generator.Generate("module2", "let y;");
    EXPECT_TRUE(result2.IsModuleDef());
}

/**
 * @test 测试生成超长参数列表
 */
TEST_F(CodeGeneratorTest, GenerateParamList_VeryLongList) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    std::vector<std::unique_ptr<Expression>> params;
    for (int i = 0; i < 1000; ++i) {
        params.push_back(std::make_unique<IntegerLiteral>(0, 0, i));
    }

    EXPECT_NO_THROW({
        generator.GenerateParamList(function_def_, params);
    });
}

/**
 * @test 测试深层嵌套作用域
 */
TEST_F(CodeGeneratorTest, DeeplyNestedScopes) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    // 创建深层嵌套的块语句
    std::vector<std::unique_ptr<Statement>>* current_statements = nullptr;
    std::unique_ptr<BlockStatement> root_block;

    for (int i = 0; i < 100; ++i) {
        auto new_statements = std::make_unique<std::vector<std::unique_ptr<Statement>>>();
        if (current_statements) {
            auto block = std::make_unique<BlockStatement>(0, 0, std::move(*current_statements));
            new_statements->push_back(std::move(block));
        }
        current_statements = new_statements.get();
        new_statements.release();
    }

    if (current_statements) {
        auto expr = std::make_unique<IntegerLiteral>(0, 0, 42);
        current_statements->push_back(std::make_unique<ExpressionStatement>(0, 0, std::move(expr)));
        std::unique_ptr<std::vector<std::unique_ptr<Statement>>> stmts_ptr(current_statements);
        auto block = std::make_unique<BlockStatement>(0, 0, std::move(*stmts_ptr));

        EXPECT_NO_THROW({
            generator.GenerateStatement(function_def_, block.get());
        });
    }
}

/**
 * @test 测试分配大量常量
 */
TEST_F(CodeGeneratorTest, AllocateLargeNumberOfConstants) {
    auto parser = CreateParser("");
    CodeGenerator generator(context_.get(), parser.get());

    const int count = 10000;
    for (int i = 0; i < count; ++i) {
        ConstIndex idx = generator.AllocateConst(Value(i));
        EXPECT_GE(idx, 0);
    }
}

} // namespace test
} // namespace compiler
} // namespace mjs

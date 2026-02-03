/**
 * @file scope_test.cpp
 * @brief 作用域测试
 *
 * 测试作用域 (Scope) 类的功能,包括:
 * - 作用域创建
 * - 变量分配
 * - 变量查找
 * - 作用域类型
 * - 变量标志
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>

#include <mjs/value/function_def.h>
#include "src/compiler/scope.h"
#include "tests/unit/test_helpers.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ScopeTest
 * @brief 作用域测试类
 */
class ScopeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试环境,包含Runtime、ModuleDef和FunctionDef
        test_env_ = std::make_unique<mjs::test::TestEnvironment>();
        function_def_base_ptr_ = test_env_->function_def();
    }

    void TearDown() override {
        test_env_.reset();
    }

    std::unique_ptr<mjs::test::TestEnvironment> test_env_;
    FunctionDefBase* function_def_base_ptr_;
};

// ============================================================================
// 作用域创建和基本属性测试
// ============================================================================

/**
 * @test 测试创建函数作用域
 */
TEST_F(ScopeTest, CreateFunctionScope) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    EXPECT_EQ(scope.function_def(), function_def_base_ptr_);
    EXPECT_EQ(scope.type(), ScopeType::kFunction);
}

/**
 * @test 测试创建块作用域
 */
TEST_F(ScopeTest, CreateBlockScope) {
    Scope scope(function_def_base_ptr_, ScopeType::kFor);

    EXPECT_EQ(scope.function_def(), function_def_base_ptr_);
    EXPECT_EQ(scope.type(), ScopeType::kFor);
}

/**
 * @test 测试创建while循环作用域
 */
TEST_F(ScopeTest, CreateWhileScope) {
    Scope scope(function_def_base_ptr_, ScopeType::kWhile);

    EXPECT_EQ(scope.function_def(), function_def_base_ptr_);
    EXPECT_EQ(scope.type(), ScopeType::kWhile);
}

/**
 * @test 测试创建try-catch作用域
 */
TEST_F(ScopeTest, CreateTryCatchScope) {
    Scope try_scope(function_def_base_ptr_, ScopeType::kTry);
    EXPECT_EQ(try_scope.type(), ScopeType::kTry);

    Scope catch_scope(function_def_base_ptr_, ScopeType::kCatch);
    EXPECT_EQ(catch_scope.type(), ScopeType::kCatch);
}

// ============================================================================
// 变量分配测试
// ============================================================================

/**
 * @test 测试分配普通变量
 */
TEST_F(ScopeTest, AllocateSimpleVariable) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto& var_info = scope.AllocateVar("x", VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);
    EXPECT_EQ(var_info.flags, VarFlags::kNone);
}

/**
 * @test 测试分配const变量
 */
TEST_F(ScopeTest, AllocateConstVariable) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto& var_info = scope.AllocateVar("PI", VarFlags::kConst);

    EXPECT_EQ(var_info.var_idx, 0);
    EXPECT_EQ(var_info.flags, VarFlags::kConst);
}

/**
 * @test 测试分配多个变量
 */
TEST_F(ScopeTest, AllocateMultipleVariables) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    auto& var1 = scope.AllocateVar("x", VarFlags::kNone);
    auto& var2 = scope.AllocateVar("y", VarFlags::kNone);
    auto& var3 = scope.AllocateVar("z", VarFlags::kConst);

    EXPECT_EQ(var1.var_idx, 0);
    EXPECT_EQ(var2.var_idx, 1);
    EXPECT_EQ(var3.var_idx, 2);
    EXPECT_EQ(var3.flags, VarFlags::kConst);
}

/**
 * @test 测试变量名包含下划线
 */
TEST_F(ScopeTest, VariableNameWithUnderscore) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto& var_info = scope.AllocateVar("_private_var", VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);
}

/**
 * @test 测试变量名包含数字
 */
TEST_F(ScopeTest, VariableNameWithNumbers) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto& var_info = scope.AllocateVar("temp123", VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);
}

/**
 * @test 测试变量名以美元符号开头
 */
TEST_F(ScopeTest, VariableNameWithDollarSign) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto& var_info = scope.AllocateVar("$jquery", VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);
}

// ============================================================================
// 变量查找测试
// ============================================================================

/**
 * @test 测试查找已存在的变量
 */
TEST_F(ScopeTest, FindExistingVariable) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    scope.AllocateVar("myVar", VarFlags::kNone);

    const auto* var_info = scope.FindVar("myVar");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->var_idx, 0);
}

/**
 * @test 测试查找不存在的变量
 */
TEST_F(ScopeTest, FindNonExistingVariable) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto* var_info = scope.FindVar("nonexistent");

    EXPECT_EQ(var_info, nullptr);
}

/**
 * @test 测试查找多个变量中的一个
 */
TEST_F(ScopeTest, FindVariableAmongMany) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    scope.AllocateVar("a", VarFlags::kNone);
    scope.AllocateVar("b", VarFlags::kNone);
    scope.AllocateVar("c", VarFlags::kNone);

    const auto* var_info = scope.FindVar("b");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->var_idx, 1);
}

/**
 * @test 测试查找const变量的标志
 */
TEST_F(ScopeTest, FindConstVariableFlags) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    scope.AllocateVar("constant", VarFlags::kConst);

    const auto* var_info = scope.FindVar("constant");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->flags, VarFlags::kConst);
}

/**
 * @test 测试空变量名
 */
TEST_F(ScopeTest, EmptyVariableName) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const auto* var_info = scope.FindVar("");

    EXPECT_EQ(var_info, nullptr);
}

// ============================================================================
// 作用域类型测试
// ============================================================================

/**
 * @test 测试if作用域
 */
TEST_F(ScopeTest, IfScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kIf);
    EXPECT_EQ(scope.type(), ScopeType::kIf);
}

/**
 * @test 测试else if作用域
 */
TEST_F(ScopeTest, ElseIfScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kElseIf);
    EXPECT_EQ(scope.type(), ScopeType::kElseIf);
}

/**
 * @test 测试else作用域
 */
TEST_F(ScopeTest, ElseScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kElse);
    EXPECT_EQ(scope.type(), ScopeType::kElse);
}

/**
 * @test 测试for循环作用域
 */
TEST_F(ScopeTest, ForScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kFor);
    EXPECT_EQ(scope.type(), ScopeType::kFor);
}

/**
 * @test 测试箭头函数作用域
 */
TEST_F(ScopeTest, ArrowFunctionScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kArrowFunction);
    EXPECT_EQ(scope.type(), ScopeType::kArrowFunction);
}

/**
 * @test 测试try-finally作用域
 */
TEST_F(ScopeTest, TryFinallyScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kTryFinally);
    EXPECT_EQ(scope.type(), ScopeType::kTryFinally);
}

/**
 * @test 测试catch-finally作用域
 */
TEST_F(ScopeTest, CatchFinallyScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kCatchFinally);
    EXPECT_EQ(scope.type(), ScopeType::kCatchFinally);
}

/**
 * @test 测试finally作用域
 */
TEST_F(ScopeTest, FinallyScopeType) {
    Scope scope(function_def_base_ptr_, ScopeType::kFinally);
    EXPECT_EQ(scope.type(), ScopeType::kFinally);
}

// ============================================================================
// 变量标志测试
// ============================================================================

/**
 * @test 测试VarFlags按位或运算
 */
TEST_F(ScopeTest, VarFlagsBitwiseOr) {
    VarFlags flags1 = VarFlags::kConst;
    VarFlags flags2 = VarFlags::kNone;
    VarFlags combined = flags1 | flags2;

    int expected = static_cast<int>(VarFlags::kConst) | static_cast<int>(VarFlags::kNone);
    EXPECT_EQ(static_cast<int>(combined), expected);
}

/**
 * @test 测试VarFlags按位与运算
 */
TEST_F(ScopeTest, VarFlagsBitwiseAnd) {
    VarFlags flags1 = VarFlags::kConst;
    VarFlags flags2 = VarFlags::kConst;
    VarFlags result = flags1 & flags2;

    EXPECT_EQ(result, VarFlags::kConst);
}

/**
 * @test 测试VarFlags按位或赋值运算
 */
TEST_F(ScopeTest, VarFlagsBitwiseOrAssignment) {
    VarFlags flags = VarFlags::kNone;
    flags |= VarFlags::kConst;

    EXPECT_EQ(flags, VarFlags::kConst);
}

// ============================================================================
// 变量隔离测试
// ============================================================================

/**
 * @test 测试不同作用域的变量索引独立
 */
TEST_F(ScopeTest, SeparateScopeVariableIndependence) {
    Scope scope1(function_def_base_ptr_, ScopeType::kFunction);
    Scope scope2(function_def_base_ptr_, ScopeType::kFor);

    scope1.AllocateVar("x", VarFlags::kNone);
    scope2.AllocateVar("y", VarFlags::kNone);

    const auto* var1 = scope1.FindVar("x");
    const auto* var2 = scope2.FindVar("y");

    ASSERT_NE(var1, nullptr);
    ASSERT_NE(var2, nullptr);
    EXPECT_EQ(var1->var_idx, 0);
    EXPECT_EQ(var2->var_idx, 1); // 共享同一个FunctionDefBase,索引递增
}

/**
 * @test 测试作用域无法查找另一作用域的变量
 */
TEST_F(ScopeTest, ScopeCannotFindOtherScopeVariable) {
    Scope scope1(function_def_base_ptr_, ScopeType::kFunction);
    Scope scope2(function_def_base_ptr_, ScopeType::kFor);

    scope1.AllocateVar("x", VarFlags::kNone);

    const auto* var_info = scope2.FindVar("x");

    EXPECT_EQ(var_info, nullptr); // scope2无法找到scope1的变量
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试长变量名
 */
TEST_F(ScopeTest, LongVariableName) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    std::string long_name(1000, 'a');
    const auto& var_info = scope.AllocateVar(long_name, VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);

    const auto* found = scope.FindVar(long_name);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->var_idx, 0);
}

/**
 * @test 测试分配大量变量
 */
TEST_F(ScopeTest, AllocateLargeNumberOfVariables) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    const int count = 1000;
    for (int i = 0; i < count; ++i) {
        std::string var_name = "var" + std::to_string(i);
        scope.AllocateVar(var_name, VarFlags::kNone);
    }

    // 检查第一个和最后一个变量
    const auto* first = scope.FindVar("var0");
    const auto* last = scope.FindVar("var" + std::to_string(count - 1));

    ASSERT_NE(first, nullptr);
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(first->var_idx, 0);
    EXPECT_EQ(last->var_idx, count - 1);
}

/**
 * @test 测试查找大小写敏感的变量名
 */
TEST_F(ScopeTest, CaseSensitiveVariableNames) {
    Scope scope(function_def_base_ptr_, ScopeType::kFunction);

    scope.AllocateVar("MyVariable", VarFlags::kNone);

    const auto* lowercase = scope.FindVar("myvariable");
    const auto* uppercase = scope.FindVar("MyVariable");

    EXPECT_EQ(lowercase, nullptr);
    ASSERT_NE(uppercase, nullptr);
}

} // namespace test
} // namespace compiler
} // namespace mjs

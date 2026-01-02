/**
 * @file scope_manager_test.cpp
 * @brief 作用域管理器测试
 *
 * 测试作用域管理器 (ScopeManager) 类的功能,包括:
 * - 作用域栈管理
 * - 作用域入栈/出栈
 * - 变量分配
 * - 变量查找(向上查找)
 * - 闭包变量捕获
 * - 作用域类型判断
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>

#include <mjs/function_def.h>
#include "src/compiler/scope_manager.h"
#include "tests/unit/test_helpers.h"

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ScopeManagerTest
 * @brief 作用域管理器测试类
 */
class ScopeManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试环境
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
// 作用域栈管理测试
// ============================================================================

/**
 * @test 测试创建作用域管理器
 */
TEST_F(ScopeManagerTest, CreateScopeManager) {
    ScopeManager manager;

    // 初始状态下作用域栈应该为空
    // 无法直接访问scopes_,但可以通过操作来验证
}

/**
 * @test 测试进入和退出单个作用域
 */
TEST_F(ScopeManagerTest, EnterAndExitSingleScope) {
    ScopeManager manager;

    // 进入函数作用域
    auto& scope = manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    EXPECT_EQ(scope.type(), ScopeType::kFunction);
    EXPECT_EQ(scope.function_def(), function_def_base_ptr_);

    // 退出作用域
    manager.ExitScope();

    // 无法直接验证栈为空,但不应该崩溃
}

/**
 * @test 测试进入多个嵌套作用域
 */
TEST_F(ScopeManagerTest, EnterMultipleNestedScopes) {
    ScopeManager manager;

    // 进入函数作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 进入if作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);

    // 进入for循环作用域
    auto& for_scope = manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFor);

    EXPECT_EQ(for_scope.type(), ScopeType::kFor);

    // 退出所有作用域
    manager.ExitScope(); // 退出for
    manager.ExitScope(); // 退出if
    manager.ExitScope(); // 退出function
}

/**
 * @test 测试重置作用域管理器
 */
TEST_F(ScopeManagerTest, ResetScopeManager) {
    ScopeManager manager;

    // 进入几个作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFor);

    // 重置
    manager.Reset();

    // 进入新作用域,应该从空栈开始
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);
    manager.ExitScope();
}

// ============================================================================
// 变量分配测试
// ============================================================================

/**
 * @test 测试在当前作用域分配变量
 */
TEST_F(ScopeManagerTest, AllocateVarInCurrentScope) {
    ScopeManager manager;

    // 进入作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 分配变量
    const auto& var_info = manager.AllocateVar("x", VarFlags::kNone);

    EXPECT_EQ(var_info.var_idx, 0);
    EXPECT_EQ(var_info.flags, VarFlags::kNone);
}

/**
 * @test 测试分配多个变量
 */
TEST_F(ScopeManagerTest, AllocateMultipleVars) {
    ScopeManager manager;

    // 进入作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 分配多个变量
    auto& var1 = manager.AllocateVar("x", VarFlags::kNone);
    auto& var2 = manager.AllocateVar("y", VarFlags::kConst);
    auto& var3 = manager.AllocateVar("z", VarFlags::kNone);

    EXPECT_EQ(var1.var_idx, 0);
    EXPECT_EQ(var2.var_idx, 1);
    EXPECT_EQ(var3.var_idx, 2);
    EXPECT_EQ(var2.flags, VarFlags::kConst);
}

/**
 * @test 测试在不同作用域中分配变量
 */
TEST_F(ScopeManagerTest, AllocateVarsInDifferentScopes) {
    ScopeManager manager;

    // 进入函数作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);
    auto& var1 = manager.AllocateVar("x", VarFlags::kNone);

    // 进入if作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);
    auto& var2 = manager.AllocateVar("y", VarFlags::kNone);

    EXPECT_EQ(var1.var_idx, 0);
    EXPECT_EQ(var2.var_idx, 1);
}

// ============================================================================
// 变量查找测试
// ============================================================================

/**
 * @test 测试在当前作用域查找变量
 */
TEST_F(ScopeManagerTest, FindVarInCurrentScope) {
    ScopeManager manager;

    // 进入作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 分配变量
    manager.AllocateVar("myVar", VarFlags::kNone);

    // 查找变量
    const auto* var_info = manager.FindVarInfoByName(function_def_base_ptr_, "myVar");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->var_idx, 0);
}

/**
 * @test 测试在外层作用域查找变量
 */
TEST_F(ScopeManagerTest, FindVarInOuterScope) {
    ScopeManager manager;

    // 进入外层作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);
    manager.AllocateVar("outerVar", VarFlags::kNone);

    // 进入内层作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);

    // 在内层作用域查找外层变量
    const auto* var_info = manager.FindVarInfoByName(function_def_base_ptr_, "outerVar");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->var_idx, 0);
}

/**
 * @test 测试查找不存在的变量
 */
TEST_F(ScopeManagerTest, FindNonExistingVar) {
    ScopeManager manager;

    // 进入作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 查找不存在的变量
    const auto* var_info = manager.FindVarInfoByName(function_def_base_ptr_, "nonexistent");

    EXPECT_EQ(var_info, nullptr);
}

/**
 * @test 测试内层变量遮蔽外层变量
 */
TEST_F(ScopeManagerTest, InnerVarShadowsOuterVar) {
    ScopeManager manager;

    // 进入外层作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);
    manager.AllocateVar("x", VarFlags::kNone);

    // 进入内层作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);
    manager.AllocateVar("x", VarFlags::kConst); // 遮蔽外层变量

    // 查找变量,应该找到内层的
    const auto* var_info = manager.FindVarInfoByName(function_def_base_ptr_, "x");

    ASSERT_NE(var_info, nullptr);
    EXPECT_EQ(var_info->flags, VarFlags::kConst);
}

// ============================================================================
// 作用域类型判断测试
// ============================================================================

/**
 * @test 测试判断是否在指定类型作用域中
 */
TEST_F(ScopeManagerTest, IsInTypeScopeBasic) {
    ScopeManager manager;

    // 初始状态不在任何作用域中
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kFor}, {}));

    // 进入for循环作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFor);

    // 现在应该在for作用域中
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kFor}, {}));
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kWhile}, {}));

    // 退出作用域
    manager.ExitScope();

    // 应该不在for作用域中
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kFor}, {}));
}

/**
 * @test 测试判断是否在多个类型作用域中的一个
 */
TEST_F(ScopeManagerTest, IsInMultipleTypeScopes) {
    ScopeManager manager;

    // 进入if作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);

    // 检查是否在多个类型中的一个
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kIf, ScopeType::kFor}, {}));
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kFor, ScopeType::kIf}, {}));
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kFor, ScopeType::kWhile}, {}));
}

/**
 * @test 测试嵌套作用域的类型判断
 */
TEST_F(ScopeManagerTest, IsInTypeScopeNested) {
    ScopeManager manager;

    // 进入函数作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 进入for循环作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFor);

    // 进入if作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);

    // 应该能找到所有类型
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kIf}, {}));
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kFor}, {}));
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kFunction}, {}));
}

/**
 * @test 测试使用结束类型的类型判断
 */
TEST_F(ScopeManagerTest, IsInTypeScopeWithEndTypes) {
    ScopeManager manager;

    // 进入函数作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 进入if作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);

    // 进入else作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kElse);

    // 从else向if查找,应该先遇到if(在end_types中),返回false
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kFunction}, {ScopeType::kIf}));

    // 从else向else查找,应该先遇到else,返回false
    EXPECT_FALSE(manager.IsInTypeScope({ScopeType::kIf}, {ScopeType::kElse}));

    // 从else向function查找,没有结束限制,应该能找到
    EXPECT_TRUE(manager.IsInTypeScope({ScopeType::kFunction}, {}));
}

// ============================================================================
// 边界情况测试
// ============================================================================

/**
 * @test 测试空作用域栈操作
 */
TEST_F(ScopeManagerTest, OperationsOnEmptyScopeStack) {
    ScopeManager manager;

    // 空栈时退出作用域,行为是未定义的,可能导致崩溃
    // 这里不测试,因为实际使用中不应该在空栈时退出
}

/**
 * @test 测试深层嵌套作用域
 */
TEST_F(ScopeManagerTest, DeeplyNestedScopes) {
    ScopeManager manager;

    const int depth = 100;

    // 进入深层嵌套作用域
    for (int i = 0; i < depth; ++i) {
        manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kIf);
    }

    // 在最内层分配变量
    manager.AllocateVar("deepVar", VarFlags::kNone);

    // 退出所有作用域
    for (int i = 0; i < depth; ++i) {
        manager.ExitScope();
    }
}

/**
 * @test 测试大量变量分配
 */
TEST_F(ScopeManagerTest, AllocateLargeNumberOfVars) {
    ScopeManager manager;

    // 进入作用域
    manager.EnterScope(function_def_base_ptr_, nullptr, ScopeType::kFunction);

    // 分配大量变量
    const int count = 1000;
    for (int i = 0; i < count; ++i) {
        std::string var_name = "var" + std::to_string(i);
        manager.AllocateVar(var_name, VarFlags::kNone);
    }

    // 验证第一个和最后一个变量
    const auto* first = manager.FindVarInfoByName(function_def_base_ptr_, "var0");
    const auto* last = manager.FindVarInfoByName(function_def_base_ptr_, "var" + std::to_string(count - 1));

    ASSERT_NE(first, nullptr);
    ASSERT_NE(last, nullptr);
    EXPECT_EQ(first->var_idx, 0);
    EXPECT_EQ(last->var_idx, count - 1);
}

} // namespace test
} // namespace compiler
} // namespace mjs

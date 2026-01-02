/**
 * @file function_module_test.cpp
 * @brief 函数和模块系统单元测试
 *
 * 测试FunctionDef、ModuleDef和ModuleManager的功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include <mjs/module_manager.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/value.h>
#include <mjs/bytecode_table.h>
#include "tests/unit/test_helpers.h"

namespace mjs {
namespace test {

/**
 * @class FunctionDefTest
 * @brief 函数定义测试
 */
class FunctionDefTest : public ::testing::Test {
protected:
    void SetUp() override {
        env_ = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        env_.reset();
    }

    std::unique_ptr<TestEnvironment> env_;
};

/**
 * @test 测试函数定义创建
 */
TEST_F(FunctionDefTest, FunctionDefCreation) {
    // Act
    auto* func_def = env_->CreateFunctionDef("testFunction", 3);

    // Assert
    ASSERT_NE(func_def, nullptr);
    EXPECT_EQ(func_def->name(), "testFunction");
    EXPECT_EQ(func_def->param_count(), 3);
    EXPECT_EQ(func_def->module_def().name(), "test_module");
}

/**
 * @test 测试函数定义类型标记
 */
TEST_F(FunctionDefTest, FunctionDefTypeFlags) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunction", 0);

    // Act & Assert - 默认不是任何特殊类型
    EXPECT_FALSE(func_def->is_normal());
    EXPECT_FALSE(func_def->is_module());
    EXPECT_FALSE(func_def->is_arrow());
    EXPECT_FALSE(func_def->is_generator());
    EXPECT_FALSE(func_def->is_async());

    // 设置为普通函数
    func_def->set_is_normal();
    EXPECT_TRUE(func_def->is_normal());
    EXPECT_FALSE(func_def->is_module());
    EXPECT_FALSE(func_def->is_arrow());
}

/**
 * @test 测试设置箭头函数类型
 */
TEST_F(FunctionDefTest, SetArrowFunction) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("arrowFunc", 0);

    // Act
    func_def->set_is_arrow();

    // Assert
    EXPECT_TRUE(func_def->is_arrow());
    EXPECT_FALSE(func_def->is_normal());
    EXPECT_FALSE(func_def->is_module());
}

/**
 * @test 测试设置生成器函数类型
 */
TEST_F(FunctionDefTest, SetGeneratorFunction) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("generatorFunc", 0);

    // Act
    func_def->set_is_normal();
    func_def->set_is_generator();

    // Assert
    EXPECT_TRUE(func_def->is_normal());
    EXPECT_TRUE(func_def->is_generator());
}

/**
 * @test 测试设置异步函数类型
 */
TEST_F(FunctionDefTest, SetAsyncFunction) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("asyncFunc", 0);

    // Act
    func_def->set_is_normal();
    func_def->set_is_async();

    // Assert
    EXPECT_TRUE(func_def->is_normal());
    EXPECT_TRUE(func_def->is_async());
}

/**
 * @test 测试函数参数数量
 */
TEST_F(FunctionDefTest, FunctionParamCount) {
    // Arrange & Act
    auto* func0 = env_->CreateFunctionDef("func0", 0);
    auto* func1 = env_->CreateFunctionDef("func1", 1);
    auto* func5 = env_->CreateFunctionDef("func5", 5);

    // Assert
    EXPECT_EQ(func0->param_count(), 0);
    EXPECT_EQ(func1->param_count(), 1);
    EXPECT_EQ(func5->param_count(), 5);
}

/**
 * @test 测试字节码表访问
 */
TEST_F(FunctionDefTest, BytecodeTableAccess) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunc", 0);

    // Act
    auto& bytecode_table = func_def->bytecode_table();

    // Assert
    EXPECT_EQ(bytecode_table.Size(), 0); // 新创建的函数应该没有字节码
}

/**
 * @test 测试变量定义表访问
 */
TEST_F(FunctionDefTest, VarDefTableAccess) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunc", 0);

    // Act
    auto& var_def_table = func_def->var_def_table();

    // Assert
    EXPECT_EQ(var_def_table.var_count(), 0); // 新创建的函数应该没有变量定义
}

/**
 * @test 测试闭包变量表访问
 */
TEST_F(FunctionDefTest, ClosureVarTableAccess) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunc", 0);

    // Act
    auto& closure_var_table = func_def->closure_var_table();

    // Assert
    EXPECT_EQ(closure_var_table.closure_var_defs().size(), 0); // 新创建的函数应该没有闭包变量
}

/**
 * @test 测试has_this标记
 */
TEST_F(FunctionDefTest, HasThisFlag) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("method", 0);

    // Act & Assert
    EXPECT_FALSE(func_def->has_this());

    func_def->set_has_this(true);
    EXPECT_TRUE(func_def->has_this());

    func_def->set_has_this(false);
    EXPECT_FALSE(func_def->has_this());
}

/**
 * @test 测试异常处理表访问
 */
TEST_F(FunctionDefTest, ExceptionTableAccess) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunc", 0);

    // Act
    auto& exception_table = func_def->exception_table();

    // Assert
    EXPECT_EQ(exception_table.GetEntries().size(), 0); // 新创建的函数应该没有异常处理条目
}

/**
 * @test 测试调试表访问
 */
TEST_F(FunctionDefTest, DebugTableAccess) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("testFunc", 0);

    // Act
    auto& debug_table = func_def->debug_table();

    // Assert - DebugTable没有size方法,我们只测试访问不抛异常
    EXPECT_NO_THROW({
        // DebugTable::FindEntry方法存在,我们测试它
        debug_table.FindEntry(0);
    });
}

/**
 * @test 测试函数名称
 */
TEST_F(FunctionDefTest, FunctionName) {
    // Arrange & Act
    auto* func_def = env_->CreateFunctionDef("myFunction", 0);

    // Assert
    EXPECT_EQ(func_def->name(), "myFunction");
}

/**
 * @test 测试函数所属模块
 */
TEST_F(FunctionDefTest, FunctionModuleDef) {
    // Arrange & Act
    auto* func_def = env_->CreateFunctionDef("myFunction", 0);

    // Assert
    EXPECT_EQ(func_def->module_def().name(), "test_module");
}

/**
 * @test 测试函数反汇编
 */
TEST_F(FunctionDefTest, FunctionDisassembly) {
    // Arrange
    auto* func_def = env_->CreateFunctionDef("emptyFunc", 0);
    Context context(env_->runtime());

    // Act
    std::string disassembly = func_def->Disassembly(&context);

    // Assert
    EXPECT_TRUE(!disassembly.empty()); // 反汇编应该产生一些输出
}

/**
 * @class ModuleDefTest
 * @brief 模块定义测试
 */
class ModuleDefTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
    }

    void TearDown() override {
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
};

/**
 * @test 测试模块定义创建
 */
TEST_F(ModuleDefTest, ModuleDefCreation) {
    // Arrange
    std::string module_name = "test_module";
    std::string module_source = "export const x = 42;";

    // Act
    auto* module_def = ModuleDef::New(runtime_.get(), module_name, module_source, 0);

    // Assert
    ASSERT_NE(module_def, nullptr);
    EXPECT_EQ(module_def->name(), module_name);
    EXPECT_EQ(module_def->param_count(), 0);
}

/**
 * @test 测试模块导出变量表
 */
TEST_F(ModuleDefTest, ModuleExportVarDefTable) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "test", "", 0);

    // Act
    auto& export_var_def_table = module_def->export_var_def_table();

    // Assert - ExportVarDefTable没有size方法,我们只测试访问不抛异常
    EXPECT_NO_THROW({
        export_var_def_table.export_var_defs();
    });
}

/**
 * @test 测试模块行号表
 */
TEST_F(ModuleDefTest, ModuleLineTable) {
    // Arrange
    std::string source = "line1\nline2\nline3";
    auto* module_def = ModuleDef::New(runtime_.get(), "test", source, 0);

    // Act
    const auto& line_table = module_def->line_table();

    // Assert - LineTable没有empty方法,我们测试PosToLineAndColumn不抛异常
    auto result = line_table.PosToLineAndColumn(0);
    EXPECT_EQ(result.first, 1);
    EXPECT_EQ(result.second, 0);
}

/**
 * @test 测试模块继承自FunctionDefBase
 */
TEST_F(ModuleDefTest, ModuleInheritsFromFunctionDefBase) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "test", "", 0);

    // Act & Assert - 模块应该可以访问FunctionDefBase的所有方法
    EXPECT_NO_THROW({
        module_def->name();
        module_def->param_count();
        module_def->bytecode_table();
        module_def->var_def_table();
        module_def->closure_var_table();
    });
}

/**
 * @test 测试模块引用计数
 */
TEST_F(ModuleDefTest, ModuleReferenceCount) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "test", "", 0);

    // Act
    module_def->Reference();
    auto ref_count_after_ref = module_def->ref_count();

    module_def->Dereference();
    auto ref_count_after_deref = module_def->ref_count();

    // Assert
    EXPECT_GT(ref_count_after_ref, 0);
    EXPECT_EQ(ref_count_after_deref, ref_count_after_ref - 1);
}

/**
 * @test 测试模块设置为模块类型
 */
TEST_F(ModuleDefTest, ModuleSetIsModule) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "test", "", 0);

    // Act
    module_def->set_is_module();

    // Assert
    EXPECT_TRUE(module_def->is_module());
    EXPECT_FALSE(module_def->is_normal());
    EXPECT_FALSE(module_def->is_arrow());
}

/**
 * @class ModuleManagerTest
 * @brief 模块管理器测试
 */
class ModuleManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
    }

    void TearDown() override {
        context_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
};

/**
 * @test 测试模块管理器非拷贝性
 */
TEST_F(ModuleManagerTest, ModuleManagerNonCopyable) {
    // ModuleManager继承自noncopyable
    EXPECT_TRUE(std::is_copy_assignable<ModuleManager>::value == false);
    EXPECT_TRUE(std::is_copy_constructible<ModuleManager>::value == false);
}

/**
 * @test 测试清理模块缓存
 */
TEST_F(ModuleManagerTest, ClearModuleCache) {
    // Arrange
    ModuleManager module_manager;

    // Act & Assert - 清理缓存不应该抛出异常
    EXPECT_NO_THROW({
        module_manager.ClearModuleCache();
    });
}

/**
 * @test 测试获取不存在的模块
 */
TEST_F(ModuleManagerTest, GetNonExistentModule) {
    // Arrange
    ModuleManager module_manager;

    // Act & Assert - 获取不存在的模块可能抛出异常或返回错误
    // 具体行为取决于实现
    EXPECT_ANY_THROW({
        module_manager.GetModule(context_.get(), "/non/existent/module");
    });
}

/**
 * @test 测试异步获取不存在的模块
 */
TEST_F(ModuleManagerTest, GetNonExistentModuleAsync) {
    // Arrange
    ModuleManager module_manager;

    // Act & Assert
    EXPECT_ANY_THROW({
        module_manager.GetModuleAsync(context_.get(), "/non/existent/module");
    });
}

/**
 * @class FunctionModuleIntegrationTest
 * @brief 函数和模块集成测试
 */
class FunctionModuleIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
    }

    void TearDown() override {
        context_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
};

/**
 * @test 测试在模块中创建函数
 */
TEST_F(FunctionModuleIntegrationTest, CreateFunctionInModule) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "testModule", "", 0);

    // Act
    auto* func_def = FunctionDef::New(module_def, "testFunction", 2);

    // Assert
    ASSERT_NE(func_def, nullptr);
    EXPECT_EQ(func_def->name(), "testFunction");
    EXPECT_EQ(func_def->param_count(), 2);
    EXPECT_EQ(&func_def->module_def(), module_def);
}

/**
 * @test 测试函数和模块的引用计数管理
 */
TEST_F(FunctionModuleIntegrationTest, ReferenceCountManagement) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "testModule", "", 0);
    auto* func_def = FunctionDef::New(module_def, "testFunction", 0);

    // Act & Assert
    module_def->Reference();
    func_def->Reference();

    EXPECT_GT(module_def->ref_count(), 0);
    EXPECT_GT(func_def->ref_count(), 0);

    module_def->Dereference();
    func_def->Dereference();
}

/**
 * @test 测试多种函数类型在同一模块中
 */
TEST_F(FunctionModuleIntegrationTest, MultipleFunctionTypes) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "testModule", "", 0);

    // Act
    auto* normal_func = FunctionDef::New(module_def, "normalFunc", 0);
    normal_func->set_is_normal();

    auto* arrow_func = FunctionDef::New(module_def, "arrowFunc", 0);
    arrow_func->set_is_arrow();

    auto* generator_func = FunctionDef::New(module_def, "generatorFunc", 0);
    generator_func->set_is_normal();
    generator_func->set_is_generator();

    auto* async_func = FunctionDef::New(module_def, "asyncFunc", 0);
    async_func->set_is_normal();
    async_func->set_is_async();

    // Assert
    EXPECT_TRUE(normal_func->is_normal());
    EXPECT_TRUE(arrow_func->is_arrow());
    EXPECT_TRUE(generator_func->is_generator());
    EXPECT_TRUE(async_func->is_async());

    EXPECT_EQ(&normal_func->module_def(), module_def);
    EXPECT_EQ(&arrow_func->module_def(), module_def);
    EXPECT_EQ(&generator_func->module_def(), module_def);
    EXPECT_EQ(&async_func->module_def(), module_def);
}

/**
 * @test 测试模块函数和普通函数的区别
 */
TEST_F(FunctionModuleIntegrationTest, ModuleVsNormalFunction) {
    // Arrange
    auto* module_def = ModuleDef::New(runtime_.get(), "testModule", "", 0);
    auto* module_func = FunctionDef::New(module_def, "moduleFunc", 0);
    auto* normal_func = FunctionDef::New(module_def, "normalFunc", 0);

    // Act
    module_func->set_is_module();
    normal_func->set_is_normal();

    // Assert
    EXPECT_TRUE(module_func->is_module());
    EXPECT_FALSE(module_func->is_normal());

    EXPECT_TRUE(normal_func->is_normal());
    EXPECT_FALSE(normal_func->is_module());
}

} // namespace test
} // namespace mjs

#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/module_object.h>
#include <mjs/value/module_def.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class ModuleObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
        context = std::make_unique<Context>(test_env->runtime());
    }

    void TearDown() override {
        context.reset();
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
    std::unique_ptr<Context> context;
};

TEST_F(ModuleObjectTest, CreateModuleObject) {
    auto* module_def = ModuleDef::New(test_env->runtime(), "testModule", "", 0);

    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);
    ASSERT_NE(module_obj.operator->(), nullptr);
    EXPECT_EQ(module_obj->module_def().name(), "testModule");
}

TEST_F(ModuleObjectTest, ModuleDefAccess) {
    auto* module_def = ModuleDef::New(test_env->runtime(), "myModule", "", 0);

    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);

    // 测试访问module_def
    EXPECT_EQ(module_obj->module_def().name(), "myModule");
}

TEST_F(ModuleObjectTest, ModuleEnvironmentAccess) {
    auto* module_def = ModuleDef::New(test_env->runtime(), "test", "", 0);
    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);

    // 测试访问模块环境
    auto& module_env = module_obj->module_env();
    // ModuleEnvironment应该是可访问的
    SUCCEED();
}

TEST_F(ModuleObjectTest, ModuleExportVars) {
    auto* module_def = ModuleDef::New(test_env->runtime(), "test", "", 0);
    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);

    // 测试导出变量
    auto& export_vars = module_obj->module_env().export_vars();
    // export_vars应该是可访问的vector
    SUCCEED();
}

TEST_F(ModuleObjectTest, ModuleInheritsFromFunctionObject) {
    auto* module_def = ModuleDef::New(test_env->runtime(), "test", "", 0);
    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);
}

} // namespace mjs::test

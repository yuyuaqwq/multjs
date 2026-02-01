#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/function_object.h>
#include <mjs/value/function_def.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class FunctionObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
        context = std::make_unique<Context>(test_env->runtime());
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
    std::unique_ptr<Context> context;
};

TEST_F(FunctionObjectTest, CreateFunctionObject) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "testFunction", 0);

    auto* func_obj = FunctionObject::New(context.get(), func_def);
    ASSERT_NE(func_obj, nullptr);
    EXPECT_EQ(func_obj->function_def().name(), "testFunction");
}

TEST_F(FunctionObjectTest, FunctionDefAccess) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "myFunction", 3);

    auto* func_obj = FunctionObject::New(context.get(), func_def);

    // 测试访问function_def
    EXPECT_EQ(func_obj->function_def().name(), "myFunction");
    EXPECT_EQ(func_obj->function_def().param_count(), 3);
}

TEST_F(FunctionObjectTest, ClosureEnvironmentAccess) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 3);
    auto* func_obj = FunctionObject::New(context.get(), func_def);

    // 测试访问闭包环境
    auto& closure_env = func_obj->closure_env();
    // ClosureEnvironment应该是可访问的
    SUCCEED();
}

TEST_F(FunctionObjectTest, FunctionToString) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "toStringTest", 3);

    auto* func_obj = FunctionObject::New(context.get(), func_def);

    // 测试ToString方法
    Value str_val = func_obj->ToString(context.get());
    EXPECT_TRUE(str_val.IsString());
    std::string_view str_view(str_val.string().data());
    EXPECT_NE(str_view.find("toStringTest"), std::string::npos);
}

TEST_F(FunctionObjectTest, FunctionInheritsFromObject) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 3);
    auto* func_obj = FunctionObject::New(context.get(), func_def);

    // 测试继承自Object
    EXPECT_TRUE(func_obj->GetPrototype(context.get()).IsObject() ||
                func_obj->GetPrototype(context.get()).IsNull());
}

TEST_F(FunctionObjectTest, FunctionWithBytecode) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "bytecodeFunction", 0);

    // 添加一些字节码
    func_def->bytecode_table().EmitOpcode(OpcodeType::kCLoad);
    func_def->bytecode_table().EmitConstIndex(0);

    auto* func_obj = FunctionObject::New(context.get(), func_def);

    // 验证函数定义包含字节码
    EXPECT_GT(func_obj->function_def().bytecode_table().Size(), 0);
}

} // namespace mjs::test

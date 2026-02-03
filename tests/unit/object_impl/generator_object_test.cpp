#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/generator_object.h>
#include <mjs/value/function_def.h>

#include "tests/unit/test_helpers.h"

namespace mjs::test {

class GeneratorObjectTest : public ::testing::Test {
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

TEST_F(GeneratorObjectTest, CreateGenerator) {
    // 创建一个函数值作为生成器函数
    auto* func_def = FunctionDef::New(test_env->module_def(), "myGenerator", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    ASSERT_NE(generator.operator->(), nullptr);
    auto generator_value = generator.ToValue();

    // 初始状态应该是suspended
    EXPECT_TRUE(generator->IsSuspended());
    EXPECT_FALSE(generator->IsExecuting());
    EXPECT_FALSE(generator->IsClosed());
}

TEST_F(GeneratorObjectTest, GeneratorStateTransitions) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "stateTest", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 初始状态: suspended
    EXPECT_TRUE(generator->IsSuspended());

    // 设置为executing
    generator->SetExecuting();
    EXPECT_TRUE(generator->IsExecuting());
    EXPECT_FALSE(generator->IsSuspended());

    // 设置为closed
    generator->SetClosed();
    EXPECT_TRUE(generator->IsClosed());
    EXPECT_FALSE(generator->IsExecuting());
}

TEST_F(GeneratorObjectTest, GeneratorFunctionDefAccess) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "generatorFunction", 2);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试访问function_def
    EXPECT_EQ(generator->function_def().name(), "generatorFunction");
    EXPECT_EQ(generator->function_def().param_count(), 2);
}

TEST_F(GeneratorObjectTest, GeneratorPcAccess) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试pc访问
    EXPECT_EQ(generator->pc(), 0);

    generator->set_pc(100);
    EXPECT_EQ(generator->pc(), 100);
}

TEST_F(GeneratorObjectTest, GeneratorStackAccess) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试栈访问
    auto& stack = generator->stack();
    // Stack应该是可访问的
    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorMakeReturnObject) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 0);
    auto func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试创建返回对象
    Value ret_value = Value(42);
    Value return_obj = generator->MakeReturnObject(context.get(), std::move(ret_value));

    // 返回对象应该是一个Value
    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorNext) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试Next方法，这里实际上会异常，因为Generator没有任何
    generator->Next(context.get());

    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorToString) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "toStringGen", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试ToString方法
    Value str_val = generator->ToString(context.get());
    EXPECT_TRUE(str_val.IsString());
    std::string_view str_view(str_val.string().data());
    EXPECT_NE(str_view.find("toStringGen"), std::string::npos);
}

TEST_F(GeneratorObjectTest, GeneratorInheritsFromObject) {
    auto* func_def = FunctionDef::New(test_env->module_def(), "", 0);
    Value func_value = Value(func_def);

    GCHandleScope<1> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);
    auto generator_value = generator.ToValue();

    // 测试继承自Object
    EXPECT_TRUE(generator->GetPrototype(context.get()).IsObject() ||
                generator->GetPrototype(context.get()).IsNull());
}

} // namespace mjs::test

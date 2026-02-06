#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/array_object.h>
#include <mjs/value/object/function_object.h>
#include <mjs/value/object/module_object.h>
#include <mjs/value/object/promise_object.h>
#include <mjs/value/object/generator_object.h>
#include <mjs/value/function_def.h>
#include <mjs/value/module_def.h>
#include "tests/unit/test_helpers.h"

namespace mjs::test {

class ObjectIntegrationTest : public ::testing::Test {
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

TEST_F(ObjectIntegrationTest, ArrayAndFunctionInterop) {
    // 创建一个包含函数的数组
    auto* func_def = FunctionDef::New(test_env->module_def(), "arrayFunc", 0);
    auto func_def_value = Value(func_def);

    GCHandleScope<2> scope(context.get());
    auto func_obj = scope.New<FunctionObject>(func_def);
    auto func_obj_value = func_obj.ToValue();

    auto arr = scope.New<ArrayObject>(std::initializer_list<Value>{
        Value(1),
        func_obj_value,
        Value(2)
    });
    auto arr_value = arr.ToValue();

    EXPECT_EQ(arr->GetLength(), 3);
    Value val;
    arr->GetComputedProperty(context.get(), Value(static_cast<int64_t>(1)), &val);
    EXPECT_TRUE(val.IsObject());
}

TEST_F(ObjectIntegrationTest, ModuleWithExports) {
    // 创建一个带有导出的模块
    auto* module_def = ModuleDef::New(test_env->runtime(), "exportModule", "", 0);
    auto module_def_value = Value(module_def);

    GCHandleScope<1> scope(context.get());
    auto module_obj = scope.New<ModuleObject>(module_def);
    auto module_obj_value = module_obj.ToValue();

    // 模块应该有导出变量环境
    EXPECT_EQ(module_obj->module_def().name(), "exportModule");
}

TEST_F(ObjectIntegrationTest, PromiseChaining) {
    // 测试Promise链(基础测试)
    auto executor = Value();

    GCHandleScope<1> scope(context.get());
    auto promise1 = scope.New<PromiseObject>(executor);
    auto promise1_value = promise1.ToValue();

    promise1->Resolve(context.get(), Value(1));

    EXPECT_TRUE(promise1->IsFulfilled());
    EXPECT_EQ(promise1->result().i64(), 1);
}

TEST_F(ObjectIntegrationTest, GeneratorAndArray) {
    // 创建生成器
    auto* func_def = FunctionDef::New(test_env->module_def(), "arrayGen", 0);
    Value func_value = Value(func_def);

    GCHandleScope<2> scope(context.get());
    auto generator = scope.New<GeneratorObject>(func_value);

    // 创建数组存储生成器
    auto arr = scope.New<ArrayObject>(std::initializer_list<Value>{
        Value(1),
        generator.ToValue(),
        Value(2)
    });
    auto arr_value = arr.ToValue();

    EXPECT_EQ(arr->GetLength(), 3);
}

} // namespace mjs::test

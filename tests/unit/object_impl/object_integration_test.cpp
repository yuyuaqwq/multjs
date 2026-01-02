#include <gtest/gtest.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value.h>
#include <mjs/object.h>
#include <mjs/object_impl/array_object.h>
#include <mjs/object_impl/function_object.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/promise_object.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
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
    auto* func_obj = FunctionObject::New(context.get(), func_def);
    auto func_obj_value = Value(func_obj);

    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(func_obj),
        Value(2)
    });
    auto arr_value = Value(arr);

    EXPECT_EQ(arr->length(), 3);
    EXPECT_TRUE((*arr)[1].IsObject());
}

TEST_F(ObjectIntegrationTest, ModuleWithExports) {
    // 创建一个带有导出的模块
    auto* module_def = ModuleDef::New(test_env->runtime(), "exportModule", "", 0);
    auto module_def_value = Value(module_def);
    auto* module_obj = ModuleObject::New(context.get(), module_def);
    auto module_obj_value = Value(module_obj);

    // 模块应该有导出变量环境
    EXPECT_EQ(module_obj->module_def().name(), "exportModule");
}

TEST_F(ObjectIntegrationTest, PromiseChaining) {
    // 测试Promise链(基础测试)
    auto executor = Value();
    auto* promise1 = PromiseObject::New(context.get(), executor);
    auto promise1_value = Value(promise1);

    promise1->Resolve(context.get(), Value(1));

    EXPECT_TRUE(promise1->IsFulfilled());
    EXPECT_EQ(promise1->result().i64(), 1);
}

TEST_F(ObjectIntegrationTest, GeneratorAndArray) {
    // 创建生成器
    auto* func_def = FunctionDef::New(test_env->module_def(), "arrayGen", 0);
    Value func_value = Value(func_def);
    auto* generator = GeneratorObject::New(context.get(), func_value);

    // 创建数组存储生成器
    auto* arr = ArrayObject::New(context.get(), {
        Value(1),
        Value(generator),
        Value(2)
    });
    auto arr_value = Value(arr);

    EXPECT_EQ(arr->length(), 3);
}

} // namespace mjs::test

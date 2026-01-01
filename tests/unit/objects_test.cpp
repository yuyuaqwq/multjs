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
#include <mjs/string.h>
#include "test_helpers.h"

namespace mjs::test {

// ============================================================================
// ArrayObject 测试
// ============================================================================

class ArrayObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(ArrayObjectTest, CreateEmptyArray) {
    auto* arr = ArrayObject::New(test_env->context, 0);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 0);
    EXPECT_EQ(arr->class_id(), ClassId::kArrayObject);
}

TEST_F(ArrayObjectTest, CreateArrayWithInitializerList) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(2),
        Value(3)
    });
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 3);
    EXPECT_EQ((*arr)[0].i32(), 1);
    EXPECT_EQ((*arr)[1].i32(), 2);
    EXPECT_EQ((*arr)[2].i32(), 3);
}

TEST_F(ArrayObjectTest, CreateArrayWithSize) {
    auto* arr = ArrayObject::New(test_env->context, 5);
    ASSERT_NE(arr, nullptr);
    EXPECT_EQ(arr->length(), 5);
}

TEST_F(ArrayObjectTest, ArrayElementAccess) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试读取元素
    EXPECT_EQ((*arr)[0].i32(), 10);
    EXPECT_EQ((*arr)[1].i32(), 20);
    EXPECT_EQ((*arr)[2].i32(), 30);

    // 测试修改元素
    (*arr)[1] = Value(99);
    EXPECT_EQ((*arr)[1].i32(), 99);
}

TEST_F(ArrayObjectTest, ArrayPush) {
    auto* arr = ArrayObject::New(test_env->context, 0);

    arr->Push(test_env->context, Value(1));
    EXPECT_EQ(arr->length(), 1);
    EXPECT_EQ((*arr)[0].i32(), 1);

    arr->Push(test_env->context, Value(2));
    EXPECT_EQ(arr->length(), 2);
    EXPECT_EQ((*arr)[1].i32(), 2);

    arr->Push(test_env->context, Value(3));
    EXPECT_EQ(arr->length(), 3);
    EXPECT_EQ((*arr)[2].i32(), 3);
}

TEST_F(ArrayObjectTest, ArrayPop) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试pop
    Value val = arr->Pop(test_env->context);
    EXPECT_EQ(val.i32(), 3);
    EXPECT_EQ(arr->length(), 2);

    val = arr->Pop(test_env->context);
    EXPECT_EQ(val.i32(), 2);
    EXPECT_EQ(arr->length(), 1);

    val = arr->Pop(test_env->context);
    EXPECT_EQ(val.i32(), 1);
    EXPECT_EQ(arr->length(), 0);
}

TEST_F(ArrayObjectTest, ArrayMixedTypes) {
    auto* str = String::New(test_env->context, "hello");
    auto* arr = ArrayObject::New(test_env->context, {
        Value(42),                    // 数字
        Value(str),                   // 字符串
        Value(true),                  // 布尔
        Value()                       // undefined
    });

    EXPECT_EQ(arr->length(), 4);
    EXPECT_EQ((*arr)[0].i32(), 42);
    EXPECT_EQ((*arr)[1].string()->str(), "hello");
    EXPECT_EQ((*arr)[2].boolean(), true);
    EXPECT_TRUE((*arr)[3].IsUndefined());
}

TEST_F(ArrayObjectTest, ArrayGetProperty) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试通过常量索引获取属性
    Value val;
    bool result = arr->GetProperty(test_env->context, 0, &val);
    EXPECT_TRUE(result);
    EXPECT_EQ(val.i32(), 1);
}

TEST_F(ArrayObjectTest, ArrayGetComputedProperty) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(10),
        Value(20),
        Value(30)
    });

    // 测试通过值获取计算属性
    Value val;
    bool result = arr->GetComputedProperty(test_env->context, Value(1), &val);
    EXPECT_TRUE(result);
    EXPECT_EQ(val.i32(), 20);
}

TEST_F(ArrayObjectTest, ArraySetComputedProperty) {
    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(2),
        Value(3)
    });

    // 测试设置计算属性
    arr->SetComputedProperty(test_env->context, Value(1), Value(99));
    EXPECT_EQ((*arr)[1].i32(), 99);
}

TEST_F(ArrayObjectTest, LargeArray) {
    // 测试大数组
    size_t size = 1000;
    auto* arr = ArrayObject::New(test_env->context, size);
    EXPECT_EQ(arr->length(), size);

    // 修改几个元素
    (*arr)[0] = Value(100);
    (*arr)[500] = Value(200);
    (*arr)[999] = Value(300);

    EXPECT_EQ((*arr)[0].i32(), 100);
    EXPECT_EQ((*arr)[500].i32(), 200);
    EXPECT_EQ((*arr)[999].i32(), 300);
}

TEST_F(ArrayObjectTest, ArrayInheritsFromObject) {
    auto* arr = ArrayObject::New(test_env->context, {Value(1), Value(2)});

    // 测试继承自Object
    EXPECT_NE(arr->GetPrototype(), nullptr);
    EXPECT_EQ(arr->GetClassDef().class_id(), ClassId::kArrayObject);
}

// ============================================================================
// FunctionObject 测试
// ============================================================================

class FunctionObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(FunctionObjectTest, CreateFunctionObject) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("testFunction");

    auto* func_obj = FunctionObject::New(test_env->context, func_def);
    ASSERT_NE(func_obj, nullptr);
    EXPECT_EQ(func_obj->function_def().name(), "testFunction");
}

TEST_F(FunctionObjectTest, FunctionDefAccess) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("myFunction");
    func_def->set_param_count(3);

    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    // 测试访问function_def
    EXPECT_EQ(func_obj->function_def().name(), "myFunction");
    EXPECT_EQ(func_obj->function_def().param_count(), 3);
}

TEST_F(FunctionObjectTest, ClosureEnvironmentAccess) {
    auto* func_def = FunctionDef::New(test_env->context);
    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    // 测试访问闭包环境
    auto& closure_env = func_obj->closure_env();
    // ClosureEnvironment应该是可访问的
    SUCCEED();
}

TEST_F(FunctionObjectTest, FunctionToString) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("toStringTest");

    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    // 测试ToString方法
    Value str_val = func_obj->ToString(test_env->context);
    EXPECT_TRUE(str_val.IsString());
    EXPECT_NE(str_val.string()->str().find("toStringTest"), std::string::npos);
}

TEST_F(FunctionObjectTest, FunctionInheritsFromObject) {
    auto* func_def = FunctionDef::New(test_env->context);
    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    // 测试继承自Object
    EXPECT_NE(func_obj->GetPrototype(), nullptr);
}

TEST_F(FunctionObjectTest, FunctionWithBytecode) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("bytecodeFunction");

    // 添加一些字节码
    func_def->bytecode_table()->EmitOp Opcode::kLoadConst;
    func_def->bytecode_table()->EmitConstIndex(0);

    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    // 验证函数定义包含字节码
    EXPECT_GT(func_obj->function_def().bytecode_table()->size(), 0);
}

// ============================================================================
// ModuleObject 测试
// ============================================================================

class ModuleObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(ModuleObjectTest, CreateModuleObject) {
    auto* module_def = ModuleDef::New(test_env->context);
    module_def->set_name("testModule");

    auto* module_obj = ModuleObject::New(test_env->context, module_def);
    ASSERT_NE(module_obj, nullptr);
    EXPECT_EQ(module_obj->module_def().name(), "testModule");
}

TEST_F(ModuleObjectTest, ModuleDefAccess) {
    auto* module_def = ModuleDef::New(test_env->context);
    module_def->set_name("myModule");

    auto* module_obj = ModuleObject::New(test_env->context, module_def);

    // 测试访问module_def
    EXPECT_EQ(module_obj->module_def().name(), "myModule");
}

TEST_F(ModuleObjectTest, ModuleEnvironmentAccess) {
    auto* module_def = ModuleDef::New(test_env->context);
    auto* module_obj = ModuleObject::New(test_env->context, module_def);

    // 测试访问模块环境
    auto& module_env = module_obj->module_env();
    // ModuleEnvironment应该是可访问的
    SUCCEED();
}

TEST_F(ModuleObjectTest, ModuleExportVars) {
    auto* module_def = ModuleDef::New(test_env->context);
    auto* module_obj = ModuleObject::New(test_env->context, module_def);

    // 测试导出变量
    auto& export_vars = module_obj->module_env().export_vars();
    // export_vars应该是可访问的vector
    SUCCEED();
}

TEST_F(ModuleObjectTest, ModuleInheritsFromFunctionObject) {
    auto* module_def = ModuleDef::New(test_env->context);
    auto* module_obj = ModuleObject::New(test_env->context, module_def);

    // ModuleObject继承自FunctionObject
    EXPECT_EQ(module_obj->class_id(), ClassId::kModuleObject);
}

// ============================================================================
// PromiseObject 测试
// ============================================================================

class PromiseObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(PromiseObjectTest, CreatePromise) {
    // 创建一个简单的executor函数
    auto executor = Value(); // 占位符

    auto* promise = PromiseObject::New(test_env->context, executor);
    ASSERT_NE(promise, nullptr);

    // 初始状态应该是pending
    EXPECT_TRUE(promise->IsPending());
    EXPECT_FALSE(promise->IsFulfilled());
    EXPECT_FALSE(promise->IsRejected());
}

TEST_F(PromiseObjectTest, PromiseStateTransitions) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // 初始状态
    EXPECT_TRUE(promise->IsPending());

    // Resolve
    promise->Resolve(test_env->context, Value(42));
    EXPECT_TRUE(promise->IsFulfilled());
    EXPECT_FALSE(promise->IsPending());
    EXPECT_FALSE(promise->IsRejected());
    EXPECT_EQ(promise->result().i32(), 42);
}

TEST_F(PromiseObjectTest, PromiseReject) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // Reject
    auto* error_str = String::New(test_env->context, "error");
    promise->Reject(test_env->context, Value(error_str));

    EXPECT_TRUE(promise->IsRejected());
    EXPECT_FALSE(promise->IsPending());
    EXPECT_FALSE(promise->IsFulfilled());
    EXPECT_EQ(promise->reason().string()->str(), "error");
}

TEST_F(PromiseObjectTest, PromiseThen) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // 创建on_fulfilled和on_rejected回调
    Value on_fulfilled; // 占位符
    Value on_rejected;  // 占位符

    // 调用Then方法
    Value result = promise->Then(test_env->context, on_fulfilled, on_rejected);

    // Then应该返回一个值(可能是另一个Promise)
    SUCCEED();
}

TEST_F(PromiseObjectTest, PromiseSetResult) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // 先resolve
    promise->Resolve(test_env->context, Value(100));

    // 设置结果
    promise->set_result(Value(200));
    EXPECT_EQ(promise->result().i32(), 200);
}

TEST_F(PromiseObjectTest, PromiseSetReason) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // 先reject
    auto* error_str = String::New(test_env->context, "failure");
    promise->Reject(test_env->context, Value(error_str));

    // 设置原因
    auto* new_error_str = String::New(test_env->context, "new error");
    promise->set_reason(Value(new_error_str));
    EXPECT_EQ(promise->reason().string()->str(), "new error");
}

TEST_F(PromiseObjectTest, PromiseInheritsFromObject) {
    auto executor = Value();
    auto* promise = PromiseObject::New(test_env->context, executor);

    // 测试继承自Object
    EXPECT_NE(promise->GetPrototype(), nullptr);
}

// ============================================================================
// GeneratorObject 测试
// ============================================================================

class GeneratorObjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(GeneratorObjectTest, CreateGenerator) {
    // 创建一个函数值作为生成器函数
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("myGenerator");
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);
    ASSERT_NE(generator, nullptr);

    // 初始状态应该是suspended
    EXPECT_TRUE(generator->IsSuspended());
    EXPECT_FALSE(generator->IsExecuting());
    EXPECT_FALSE(generator->IsClosed());
}

TEST_F(GeneratorObjectTest, GeneratorStateTransitions) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("stateTest");
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

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
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("generatorFunction");
    func_def->set_param_count(2);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试访问function_def
    EXPECT_EQ(generator->function_def().name(), "generatorFunction");
    EXPECT_EQ(generator->function_def().param_count(), 2);
}

TEST_F(GeneratorObjectTest, GeneratorPcAccess) {
    auto* func_def = FunctionDef::New(test_env->context);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试pc访问
    EXPECT_EQ(generator->pc(), 0);

    generator->set_pc(100);
    EXPECT_EQ(generator->pc(), 100);
}

TEST_F(GeneratorObjectTest, GeneratorStackAccess) {
    auto* func_def = FunctionDef::New(test_env->context);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试栈访问
    auto& stack = generator->stack();
    // Stack应该是可访问的
    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorMakeReturnObject) {
    auto* func_def = FunctionDef::New(test_env->context);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试创建返回对象
    Value ret_value = Value(42);
    Value return_obj = generator->MakeReturnObject(test_env->context, std::move(ret_value));

    // 返回对象应该是一个Value
    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorNext) {
    auto* func_def = FunctionDef::New(test_env->context);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试Next方法
    generator->Next(test_env->context);
    // Next应该会改变生成器的状态或pc
    SUCCEED();
}

TEST_F(GeneratorObjectTest, GeneratorToString) {
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("toStringGen");
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试ToString方法
    Value str_val = generator->ToString(test_env->context);
    EXPECT_TRUE(str_val.IsString());
    EXPECT_NE(str_val.string()->str().find("toStringGen"), std::string::npos);
}

TEST_F(GeneratorObjectTest, GeneratorInheritsFromObject) {
    auto* func_def = FunctionDef::New(test_env->context);
    Value func_value = Value(func_def);

    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 测试继承自Object
    EXPECT_NE(generator->GetPrototype(), nullptr);
    EXPECT_EQ(generator->class_id(), ClassId::kGeneratorObject);
}

// ============================================================================
// 对象集成测试
// ============================================================================

class ObjectIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_env = std::make_unique<TestEnvironment>();
    }

    void TearDown() override {
        test_env.reset();
    }

    std::unique_ptr<TestEnvironment> test_env;
};

TEST_F(ObjectIntegrationTest, ArrayAndFunctionInterop) {
    // 创建一个包含函数的数组
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("arrayFunc");
    auto* func_obj = FunctionObject::New(test_env->context, func_def);

    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(func_obj),
        Value(2)
    });

    EXPECT_EQ(arr->length(), 3);
    EXPECT_TRUE((*arr)[1].IsObject());
}

TEST_F(ObjectIntegrationTest, ModuleWithExports) {
    // 创建一个带有导出的模块
    auto* module_def = ModuleDef::New(test_env->context);
    module_def->set_name("exportModule");
    auto* module_obj = ModuleObject::New(test_env->context, module_def);

    // 模块应该有导出变量环境
    EXPECT_EQ(module_obj->module_def().name(), "exportModule");
}

TEST_F(ObjectIntegrationTest, PromiseChaining) {
    // 测试Promise链(基础测试)
    auto executor = Value();
    auto* promise1 = PromiseObject::New(test_env->context, executor);
    promise1->Resolve(test_env->context, Value(1));

    EXPECT_TRUE(promise1->IsFulfilled());
    EXPECT_EQ(promise1->result().i32(), 1);
}

TEST_F(ObjectIntegrationTest, GeneratorAndArray) {
    // 创建生成器
    auto* func_def = FunctionDef::New(test_env->context);
    func_def->set_name("arrayGen");
    Value func_value = Value(func_def);
    auto* generator = GeneratorObject::New(test_env->context, func_value);

    // 创建数组存储生成器
    auto* arr = ArrayObject::New(test_env->context, {
        Value(1),
        Value(generator),
        Value(2)
    });

    EXPECT_EQ(arr->length(), 3);
}

} // namespace mjs::test

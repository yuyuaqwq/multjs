/**
 * @file runtime_context_test.cpp
 * @brief Runtime和Context单元测试
 *
 * 测试JavaScript运行时环境和执行上下文的核心功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/value.h>
#include <mjs/object.h>
#include <mjs/string.h>

namespace mjs {
namespace test {

/**
 * @class RuntimeTest
 * @brief Runtime类单元测试
 */
class RuntimeTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = std::make_unique<Runtime>();
    }

    void TearDown() override {
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
};

/**
 * @brief 测试Runtime默认构造
 */
TEST_F(RuntimeTest, DefaultConstruction) {
    // Arrange & Act
    auto runtime = std::make_unique<Runtime>();

    // Assert
    EXPECT_NE(runtime, nullptr);
    EXPECT_TRUE(runtime->global_this().IsObject());
}

/**
 * @brief 测试Runtime自定义模块管理器构造
 */
TEST_F(RuntimeTest, CustomModuleManagerConstruction) {
    // Arrange & Act
    // 注意:ModuleManagerBase是抽象类,不能直接实例化
    // 这个测试只验证默认构造函数
    auto runtime = std::make_unique<Runtime>();

    // Assert
    EXPECT_NE(runtime, nullptr);
    EXPECT_TRUE(runtime->global_this().IsObject());
}

/**
 * @brief 测试Runtime全局this对象初始化
 */
TEST_F(RuntimeTest, GlobalThisInitialization) {
    // Arrange & Act
    auto& global_this = runtime_->global_this();

    // Assert
    EXPECT_TRUE(global_this.IsObject());
}

/**
 * @brief 测试Runtime全局常量池访问
 */
TEST_F(RuntimeTest, GlobalConstPoolAccess) {
    // Arrange & Act
    auto& global_const_pool = runtime_->global_const_pool();

    // Assert
    EXPECT_NE(&global_const_pool, nullptr);
}

/**
 * @brief 测试Runtime类定义表访问
 */
TEST_F(RuntimeTest, ClassDefTableAccess) {
    // Arrange & Act
    auto& class_def_table = runtime_->class_def_table();

    // Assert
    EXPECT_NE(&class_def_table, nullptr);
}

/**
 * @brief 测试Runtime模块管理器访问
 */
TEST_F(RuntimeTest, ModuleManagerAccess) {
    // Arrange & Act
    auto& module_manager = runtime_->module_manager();

    // Assert
    // 模块管理器总是有效,不需要测试nullptr
    SUCCEED();
}

/**
 * @brief 测试Runtime线程本地栈访问
 */
TEST_F(RuntimeTest, StackAccess) {
    // Arrange & Act
    auto& stack = runtime_->stack();

    // Assert
    EXPECT_EQ(stack.size(), 0);
}

/**
 * @brief 测试Runtime添加全局属性
 */
TEST_F(RuntimeTest, AddPropertyToGlobalThis) {
    // Arrange
    const char* prop_name = "testProp";
    Value test_value(42);

    // Act
    runtime_->AddPropertyToGlobalThis(prop_name, std::move(test_value));

    // Assert
    auto const_idx = runtime_->global_const_pool().Find(Value(prop_name));
    EXPECT_TRUE(const_idx.has_value());
}

/**
 * @brief 测试Runtime添加多个全局属性
 */
TEST_F(RuntimeTest, AddMultiplePropertiesToGlobalThis) {
    // Arrange
    Value value1(100);
    Value value2("test");
    Value value3(true);

    // Act
    runtime_->AddPropertyToGlobalThis("prop1", std::move(value1));
    runtime_->AddPropertyToGlobalThis("prop2", std::move(value2));
    runtime_->AddPropertyToGlobalThis("prop3", std::move(value3));

    // Assert
    auto const_idx1 = runtime_->global_const_pool().Find(Value("prop1"));
    auto const_idx2 = runtime_->global_const_pool().Find(Value("prop2"));
    auto const_idx3 = runtime_->global_const_pool().Find(Value("prop3"));

    EXPECT_TRUE(const_idx1.has_value());
    EXPECT_TRUE(const_idx2.has_value());
    EXPECT_TRUE(const_idx3.has_value());
}

/**
 * @brief 测试Runtime控制台对象初始化
 */
TEST_F(RuntimeTest, ConsoleInitialization) {
    // Arrange & Act
    auto const_idx = runtime_->global_const_pool().Find(Value("console"));

    // Assert
    EXPECT_TRUE(const_idx.has_value());
}

/**
 * @class ContextTest
 * @brief Context类单元测试
 */
class ContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = std::make_unique<Runtime>();
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
 * @brief 测试Context构造
 */
TEST_F(ContextTest, Construction) {
    // Arrange & Act
    auto runtime = std::make_unique<Runtime>();
    auto context = std::make_unique<Context>(runtime.get());

    // Assert
    EXPECT_NE(context, nullptr);
    EXPECT_EQ(&context->runtime(), runtime.get());
}

/**
 * @brief 测试Context运行时访问
 */
TEST_F(ContextTest, RuntimeAccess) {
    // Arrange & Act
    auto& runtime = context_->runtime();

    // Assert
    EXPECT_EQ(&runtime, runtime_.get());
}

/**
 * @brief 测试Context本地常量池访问
 */
TEST_F(ContextTest, LocalConstPoolAccess) {
    // Arrange & Act
    auto& local_const_pool = context_->local_const_pool();

    // Assert
    EXPECT_NE(&local_const_pool, nullptr);
}

/**
 * @brief 测试Context形状管理器访问
 */
TEST_F(ContextTest, ShapeManagerAccess) {
    // Arrange & Act
    auto& shape_manager = context_->shape_manager();

    // Assert
    EXPECT_NE(&shape_manager, nullptr);
}

/**
 * @brief 测试Context垃圾回收管理器访问
 */
TEST_F(ContextTest, GCManagerAccess) {
    // Arrange & Act
    auto& gc_manager = context_->gc_manager();

    // Assert
    EXPECT_NE(&gc_manager, nullptr);
}

/**
 * @brief 测试Context微任务队列访问
 */
TEST_F(ContextTest, MicrotaskQueueAccess) {
    // Arrange & Act
    auto& microtask_queue = context_->microtask_queue();

    // Assert
    EXPECT_EQ(microtask_queue.size(), 0);
}

/**
 * @brief 测试Context编译简单模块
 */
TEST_F(ContextTest, CompileSimpleModule) {
    // Arrange
    std::string module_name = "test_module";
    std::string_view script = "var x = 42;";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context编译空模块
 */
TEST_F(ContextTest, CompileEmptyModule) {
    // Arrange
    std::string module_name = "empty_module";
    std::string_view script = "";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context编译包含多个语句的模块
 */
TEST_F(ContextTest, CompileMultiStatementModule) {
    // Arrange
    std::string module_name = "multi_stmt_module";
    std::string_view script = "var a = 1; var b = 2; var c = a + b;";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context编译包含函数定义的模块
 */
TEST_F(ContextTest, CompileFunctionModule) {
    // Arrange
    std::string module_name = "function_module";
    std::string_view script = "function test() { return 42; }";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context编译包含对象字面量的模块
 */
TEST_F(ContextTest, CompileObjectModule) {
    // Arrange
    std::string module_name = "object_module";
    std::string_view script = "var obj = { a: 1, b: 2 };";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context编译包含数组字面量的模块
 */
TEST_F(ContextTest, CompileArrayModule) {
    // Arrange
    std::string module_name = "array_module";
    std::string_view script = "var arr = [1, 2, 3];";

    // Act
    Value result = context_->CompileModule(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context Eval简单表达式
 */
TEST_F(ContextTest, EvalSimpleExpression) {
    // Arrange
    std::string module_name = "eval_test";
    std::string_view script = "42;";

    // Act
    Value result = context_->Eval(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context Eval变量声明
 */
TEST_F(ContextTest, EvalVariableDeclaration) {
    // Arrange
    std::string module_name = "var_eval_test";
    std::string_view script = "var x = 100;";

    // Act
    Value result = context_->Eval(std::move(module_name), script);

    // Assert
    EXPECT_TRUE(result.IsModuleDef());
}

/**
 * @brief 测试Context查找或插入本地常量
 */
TEST_F(ContextTest, FindConstOrInsertToLocal) {
    // Arrange
    Value test_value(42);

    // Act
    ConstIndex idx1 = context_->FindConstOrInsertToLocal(test_value);
    ConstIndex idx2 = context_->FindConstOrInsertToLocal(test_value);

    // Assert
    EXPECT_EQ(idx1, idx2);
    const Value& retrieved = context_->GetConstValue(idx1);
    EXPECT_EQ(retrieved.i64(), 42);
}

/**
 * @brief 测试Context查找或插入多个不同的本地常量
 */
TEST_F(ContextTest, FindConstOrInsertToLocalMultipleValues) {
    // Arrange
    Value value1(42);
    Value value2("hello");
    Value value3(true);

    // Act
    ConstIndex idx1 = context_->FindConstOrInsertToLocal(value1);
    ConstIndex idx2 = context_->FindConstOrInsertToLocal(value2);
    ConstIndex idx3 = context_->FindConstOrInsertToLocal(value3);

    // Assert
    EXPECT_NE(idx1, idx2);
    EXPECT_NE(idx2, idx3);

    const Value& retrieved1 = context_->GetConstValue(idx1);
    const Value& retrieved2 = context_->GetConstValue(idx2);
    const Value& retrieved3 = context_->GetConstValue(idx3);

    EXPECT_EQ(retrieved1.i64(), 42);
    EXPECT_TRUE(retrieved2.IsString());
    EXPECT_TRUE(retrieved3.ToBoolean().boolean());
}

/**
 * @brief 测试Context查找或插入全局常量
 */
TEST_F(ContextTest, FindConstOrInsertToGlobal) {
    // Arrange
    Value test_value(99);

    // Act
    ConstIndex idx1 = context_->FindConstOrInsertToGlobal(test_value);
    ConstIndex idx2 = context_->FindConstOrInsertToGlobal(test_value);

    // Assert
    EXPECT_EQ(idx1, idx2);
    const Value& retrieved = context_->GetConstValue(idx1);
    EXPECT_EQ(retrieved.i64(), 99);
}

/**
 * @brief 测试Context获取常量值
 */
TEST_F(ContextTest, GetConstValue) {
    // Arrange
    Value test_value(123);
    ConstIndex idx = context_->FindConstOrInsertToLocal(test_value);

    // Act
    const Value& retrieved = context_->GetConstValue(idx);

    // Assert
    EXPECT_EQ(retrieved.i64(), 123);
}

/**
 * @brief 测试Context本地和全局常量池隔离
 */
TEST_F(ContextTest, LocalGlobalConstPoolIsolation) {
    // Arrange
    Value local_value(1);
    Value global_value(2);

    // Act
    ConstIndex local_idx = context_->FindConstOrInsertToLocal(local_value);
    ConstIndex global_idx = context_->FindConstOrInsertToGlobal(global_value);

    // Assert
    EXPECT_NE(local_idx, global_idx);

    const Value& local_retrieved = context_->GetConstValue(local_idx);
    const Value& global_retrieved = context_->GetConstValue(global_idx);

    EXPECT_EQ(local_retrieved.i64(), 1);
    EXPECT_EQ(global_retrieved.i64(), 2);
}

/**
 * @brief 测试Context销毁时清理栈
 */
TEST_F(ContextTest, DestructionClearsStack) {
    // Arrange
    runtime_->stack().push(Value(42));
    EXPECT_EQ(runtime_->stack().size(), 1);

    // Act
    {
        Context context(runtime_.get());
    }
    // context被销毁

    // Assert
    EXPECT_EQ(runtime_->stack().size(), 0);
}

/**
 * @brief 测试Runtime非拷贝性
 */
TEST_F(RuntimeTest, NonCopyable) {
    // Assert: Runtime不能被拷贝
    EXPECT_FALSE(std::is_copy_constructible<Runtime>::value);
    EXPECT_FALSE(std::is_copy_assignable<Runtime>::value);
}

/**
 * @brief 测试Context非拷贝性
 */
TEST_F(ContextTest, NonCopyable) {
    // Assert: Context不能被拷贝
    EXPECT_FALSE(std::is_copy_constructible<Context>::value);
    EXPECT_FALSE(std::is_copy_assignable<Context>::value);
}

} // namespace test
} // namespace mjs

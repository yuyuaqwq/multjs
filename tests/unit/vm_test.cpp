/**
 * @file vm_test.cpp
 * @brief 虚拟机(VM)单元测试
 *
 * 测试VM的所有核心功能，包括：
 * - 基础操作（变量读写、常量加载）
 * - 模块初始化和绑定
 * - 闭包创建和绑定
 * - 函数调度
 * - 字节码执行
 * - 异常处理
 * - 生成器和异步函数支持
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <mjs/vm.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value.h>
#include <mjs/stack_frame.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include <mjs/object.h>
#include <mjs/object_impl/function_object.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/object_impl/async_object.h>
#include <mjs/object_impl/promise_object.h>
#include <mjs/object_impl/constructor_object.h>
#include <mjs/bytecode_table.h>
#include <mjs/variable.h>
#include <mjs/closure.h>
#include <mjs/exception.h>

#include "tests/unit/test_helpers.h"

namespace mjs {
namespace test {

// =============================================================================
// VM基础测试
// =============================================================================

/**
 * @class VMTest
 * @brief VM基础测试类
 */
class VMTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 2);
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        module_def_.reset();
        function_def_.reset();
        runtime_.reset();
    }

    Value& GetVar(VM* vm, StackFrame* stack_frame, VarIndex var_index) {
        return vm->GetVar(*stack_frame, var_index);
    }

    void SetVar(VM* vm, StackFrame* stack_frame, VarIndex var_index, Value&& var) {
        return vm->SetVar(stack_frame, var_index, std::forward<Value>(var));
    }

    bool FunctionScheduling(VM* vm, StackFrame* stack_frame, uint32_t param_count) {
        return vm->FunctionScheduling(stack_frame, param_count);
    }

    void LoadConst(VM* vm, StackFrame* stack_frame, ConstIndex const_idx) {
        vm->LoadConst(stack_frame, const_idx);
    }

    bool ThrowException(VM* vm, StackFrame* stack_frame, std::optional<Value>* error_val) {
        return vm->ThrowException(stack_frame, error_val);
    }

    void GeneratorSaveContext(VM* vm, StackFrame* stack_frame, GeneratorObject* generator) {
        vm->GeneratorSaveContext(stack_frame, generator);
    }

    void GeneratorRestoreContext(VM* vm, StackFrame* stack_frame, GeneratorObject* generator) {
        vm->GeneratorRestoreContext(stack_frame, generator);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试VM构造函数
 */
TEST_F(VMTest, VMConstruction) {
    // Arrange & Act
    VM vm(context_.get());

    // Assert - VM成功构造，无异常抛出
    SUCCEED();
}

/**
 * @test 测试GetVar - 普通变量
 */
TEST_F(VMTest, GetVar_NormalVariable) {
    // Arrange
    VM vm(context_.get());
    stack_frame_->push(Value(42));
    stack_frame_->push(Value(100));

    // Act
    auto& value = GetVar(&vm, stack_frame_.get(), 0);

    // Assert
    EXPECT_EQ(value.i64(), 42);
}

/**
 * @test 测试GetVar - 闭包变量
 */
TEST_F(VMTest, GetVar_ClosureVariable) {
    // Arrange
    VM vm(context_.get());
    auto* closure_var = new ClosureVar(Value(42));
    stack_frame_->push(Value(closure_var));

    // Act
    auto& value = GetVar(&vm, stack_frame_.get(), 0);

    // Assert
    EXPECT_EQ(value.i64(), 42);
}

/**
 * @test 测试SetVar - 普通变量
 */
TEST_F(VMTest, SetVar_NormalVariable) {
    // Arrange
    VM vm(context_.get());
    stack_frame_->push(Value(0));
    stack_frame_->push(Value(0));

    // Act
    SetVar(&vm, stack_frame_.get(), 0, Value(42));

    // Assert
    EXPECT_EQ(stack_frame_->get(0).i64(), 42);
}

/**
 * @test 测试SetVar - 闭包变量
 */
TEST_F(VMTest, SetVar_ClosureVariable) {
    // Arrange
    VM vm(context_.get());
    auto* closure_var = new ClosureVar(Value(0));
    stack_frame_->push(Value(closure_var));

    // Act
    SetVar(&vm, stack_frame_.get(), 0, Value(42));

    // Assert
    EXPECT_EQ(closure_var->value().i64(), 42);
}

/**
 * @test 测试SetVar - 导出变量
 */
TEST_F(VMTest, SetVar_ExportVariable) {
    // Arrange
    VM vm(context_.get());
    auto* export_var = new ExportVar(Value(0));
    stack_frame_->push(Value(export_var));

    // Act
    SetVar(&vm, stack_frame_.get(), 0, Value(42));

    // Assert
    EXPECT_EQ(export_var->value().i64(), 42);
}

/**
 * @test 测试GetVar - 导出变量
 */
TEST_F(VMTest, GetVar_ExportVariable) {
    // Arrange
    VM vm(context_.get());
    auto* export_var = new ExportVar(Value(42));
    stack_frame_->push(Value(export_var));

    // Act
    auto& value = GetVar(&vm, stack_frame_.get(), 0);

    // Assert
    EXPECT_EQ(value.i64(), 42);
}

// =============================================================================
// 模块初始化和绑定测试
// =============================================================================

/**
 * @class VMModuleTest
 * @brief VM模块相关测试
 */
class VMModuleTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
};

/**
 * @test 测试ModuleInit - 无导出变量的模块
 */
TEST_F(VMModuleTest, ModuleInit_NoExports) {
    // Arrange
    VM vm(context_.get());
    Value module_val(module_def_.get());

    // Act
    vm.ModuleInit(&module_val);

    // Assert - 模块值应该保持为ModuleDef类型
    EXPECT_TRUE(module_val.IsModuleDef());
}

/**
 * @test 测试ModuleInit - 有导出变量的模块
 */
TEST_F(VMModuleTest, ModuleInit_WithExports) {
    // Arrange
    VM vm(context_.get());
    auto& export_table = module_def_->export_var_def_table();
    export_table.AddExportVar("export1", 0);
    export_table.AddExportVar("export2", 1);

    Value module_val(module_def_.get());

    // Act
    vm.ModuleInit(&module_val);

    // Assert - 模块值应该变成ModuleObject类型
    EXPECT_TRUE(module_val.IsModuleObject());

    // 获取 ModuleObject 引用并设置导出变量
    auto& module_obj = module_val.module();
    module_obj.module_env().export_vars().resize(2);
    module_obj.module_env().export_vars()[0] = ExportVar(Value(42));
    module_obj.module_env().export_vars()[1] = ExportVar(Value(100));
}

/**
 * @test 测试BindModuleExportVars
 */
TEST_F(VMModuleTest, BindModuleExportVars) {
    // Arrange
    VM vm(context_.get());
    auto& export_table = module_def_->export_var_def_table();
    export_table.AddExportVar("export1", 0);
    export_table.AddExportVar("export2", 1);

    auto* module_obj = ModuleObject::New(context_.get(), module_def_.get());
    module_obj->module_env().export_vars().resize(2);
    module_obj->module_env().export_vars()[0] = ExportVar(Value(42));
    module_obj->module_env().export_vars()[1] = ExportVar(Value(100));

    stack_frame_->set_function_val(Value(module_obj));
    stack_frame_->upgrade(2);  // 为2个导出变量预留空间

    // Act
    vm.BindModuleExportVars(stack_frame_.get());

    // Assert - 栈帧上的变量应该绑定到导出变量
    EXPECT_TRUE(stack_frame_->get(0).IsExportVar());
    EXPECT_TRUE(stack_frame_->get(1).IsExportVar());
}

// =============================================================================
// 闭包相关测试
// =============================================================================

/**
 * @class VMClosureTest
 * @brief VM闭包相关测试
 */
class VMClosureTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 0);
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    void Closure(VM* vm, const StackFrame& stack_frame, Value* value) {
        vm->Closure(stack_frame, value);
    }

    void BindClosureVars(VM* vm, StackFrame* stack_frame) {
        vm->BindClosureVars(stack_frame);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试Closure - 创建闭包并捕获变量
 */
TEST_F(VMClosureTest, Closure_CreateWithCapturedVars) {
    // Arrange
    VM vm(context_.get());
    function_def_->closure_var_table().AddClosureVar(0, 0);
    function_def_->set_has_this(true);
    function_def_->set_is_arrow();

    stack_frame_->push(Value(42));  // 要捕获的变量
    stack_frame_->set_this_val(Value(Object::New(context_.get())));

    Value func_val(function_def_.get());

    // Act
    Closure(&vm, *stack_frame_, &func_val);

    // Assert
    EXPECT_TRUE(func_val.IsFunctionObject());
    EXPECT_TRUE(func_val.function().closure_env().closure_var_refs().size() > 0);
}

/**
 * @test 测试BindClosureVars
 */
TEST_F(VMClosureTest, BindClosureVars) {
    // Arrange
    VM vm(context_.get());
    // 添加局部变量定义和闭包变量定义(必须在创建FunctionObject之前)
    function_def_->var_def_table().AddVar("local");
    function_def_->closure_var_table().AddClosureVar(0, 0);

    auto* func_obj = FunctionObject::New(context_.get(), function_def_.get());
    // FunctionObject构造函数已经自动resize了closure_var_refs,现在只需设置值
    func_obj->closure_env().closure_var_refs()[0] = Value(new ClosureVar(Value(42)));

    stack_frame_->set_function_val(Value(func_obj));
    stack_frame_->set_function_def(function_def_.get());
    stack_frame_->upgrade(1);

    // Act
    BindClosureVars(&vm, stack_frame_.get());

    // Assert - 栈帧上的变量应该绑定到闭包变量
    EXPECT_TRUE(stack_frame_->get(0).IsClosureVar());
}

// =============================================================================
// 函数调度测试
// =============================================================================

/**
 * @class VMFunctionSchedulingTest
 * @brief VM函数调度测试
 */
class VMFunctionSchedulingTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 2);
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    bool FunctionScheduling(VM* vm, StackFrame* stack_frame, uint32_t param_count) {
        return vm->FunctionScheduling(stack_frame, param_count);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试FunctionScheduling - FunctionDef类型
 */
TEST_F(VMFunctionSchedulingTest, FunctionScheduling_FunctionDef) {
    // Arrange
    VM vm(context_.get());
    stack_frame_->set_function_val(Value(function_def_.get()));
    stack_frame_->push(Value(1));  // 参数1
    stack_frame_->push(Value(2));  // 参数2

    // Act
    bool continue_exec = FunctionScheduling(&vm, stack_frame_.get(), 2);

    // Assert
    EXPECT_TRUE(continue_exec);
    EXPECT_EQ(stack_frame_->function_def(), function_def_.get());
}

/**
 * @test 测试FunctionScheduling - 参数不足
 */
TEST_F(VMFunctionSchedulingTest, FunctionScheduling_NotEnoughParameters) {
    // Arrange
    VM vm(context_.get());
    stack_frame_->set_function_val(Value(function_def_.get()));
    stack_frame_->push(Value(1));  // 只有1个参数，但需要2个

    // Act
    bool continue_exec = FunctionScheduling(&vm, stack_frame_.get(), 1);

    // Assert
    EXPECT_FALSE(continue_exec);
    EXPECT_TRUE(stack_frame_->get(-1).IsException());
}

/**
 * @test 测试FunctionScheduling - 生成器函数
 */
TEST_F(VMFunctionSchedulingTest, FunctionScheduling_GeneratorFunction) {
    // Arrange
    VM vm(context_.get());
    function_def_->set_is_generator();
    stack_frame_->set_function_val(Value(function_def_.get()));
    stack_frame_->push(Value(1));
    stack_frame_->push(Value(2));

    // Act
    bool continue_exec = FunctionScheduling(&vm, stack_frame_.get(), 2);

    // Assert
    EXPECT_FALSE(continue_exec);
    EXPECT_TRUE(stack_frame_->get(-1).IsGeneratorObject());
}

/**
 * @test 测试FunctionScheduling - 异步函数
 */
TEST_F(VMFunctionSchedulingTest, FunctionScheduling_AsyncFunction) {
    // Arrange
    VM vm(context_.get());
    function_def_->set_is_async();
    stack_frame_->set_function_val(Value(function_def_.get()));
    stack_frame_->push(Value(1));
    stack_frame_->push(Value(2));

    // Act
    bool continue_exec = FunctionScheduling(&vm, stack_frame_.get(), 2);

    // Assert
    EXPECT_TRUE(continue_exec);
    EXPECT_TRUE(stack_frame_->function_val().IsAsyncObject());
}

/**
 * @test 测试FunctionScheduling - CppFunction类型
 */
TEST_F(VMFunctionSchedulingTest, FunctionScheduling_CppFunction) {
    // Arrange
    VM vm(context_.get());
    Value::CppFunction cpp_func = [](Context* ctx, uint32_t param_count, const StackFrame& stack_frame) -> Value {
        return Value(42);
    };
    stack_frame_->set_function_val(Value(cpp_func));
    stack_frame_->push(Value(1));

    // Act
    bool continue_exec = FunctionScheduling(&vm, stack_frame_.get(), 1);

    // Assert
    EXPECT_FALSE(continue_exec);
    EXPECT_EQ(stack_frame_->get(-1).i64(), 42);
}

// =============================================================================
// 字节码执行测试
// =============================================================================

/**
 * @class VMBytecodeExecutionTest
 * @brief VM字节码执行测试
 */
class VMBytecodeExecutionTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 0);

        // 设置函数定义的字节码
        function_def_->bytecode_table().EmitOpcode(OpcodeType::kReturn);
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    // 辅助函数：添加常量到常量池
    ConstIndex AddConstant(const Value& value) {
        return context_->FindConstOrInsertToGlobal(value);
    }

    // 辅助函数：根据常量索引发出加载常量指令
    void EmitLoadConst(BytecodeTable& bytecode_table, ConstIndex const_idx) {
        if (const_idx <= 5) {
            bytecode_table.EmitOpcode(OpcodeType::kCLoad_0 + const_idx);
        } else {
            bytecode_table.EmitOpcode(OpcodeType::kCLoadD);
            bytecode_table.EmitU32(const_idx);
        }
    }

    void LoadConst(VM* vm, StackFrame* stack_frame, ConstIndex const_idx) {
        vm->LoadConst(stack_frame, const_idx);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试LoadConst
 */
TEST_F(VMBytecodeExecutionTest, LoadConst_Operation) {
    // Arrange
    VM vm(context_.get());
    ConstIndex const_idx = AddConstant(Value(42));

    // Act
    LoadConst(&vm, stack_frame_.get(), const_idx);

    // Assert
    EXPECT_EQ(stack_frame_->get(-1).i64(), 42);
}

/**
 * @test 测试CallFunction - 简单函数调用
 */
TEST_F(VMBytecodeExecutionTest, CallFunction_SimpleCall) {
    // Arrange
    VM vm(context_.get());
    auto* simple_func = TestFunctionDef::Create(module_def_.get(), "simple", 0);

    Value const_val(42);
    ConstIndex const_idx = AddConstant(const_val);

    // 根据常量索引选择合适的加载指令
    if (const_idx <= 5) {
        simple_func->bytecode_table().EmitOpcode(OpcodeType::kCLoad_0 + const_idx);
    } else {
        simple_func->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
        simple_func->bytecode_table().EmitU32(const_idx);
    }
    simple_func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(simple_func);

    std::vector<Value> args;

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 42);
}

/**
 * @test 测试CallFunction - 带参数的函数调用
 */
TEST_F(VMBytecodeExecutionTest, CallFunction_WithParameters) {
    // Arrange
    VM vm(context_.get());
    auto* add_func = TestFunctionDef::Create(module_def_.get(), "add", 2);
    add_func->bytecode_table().EmitOpcode(OpcodeType::kVLoad_0);  // 加载参数0
    add_func->bytecode_table().EmitOpcode(OpcodeType::kVLoad_1);  // 加载参数1
    add_func->bytecode_table().EmitOpcode(OpcodeType::kAdd);      // 相加
    add_func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(add_func);
    std::vector<Value> args = {Value(10), Value(32)};

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 42);
}

// =============================================================================
// 异常处理测试
// =============================================================================

/**
 * @class VMExceptionTest
 * @brief VM异常处理测试
 */
class VMExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 0);
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    bool ThrowException(VM* vm, StackFrame* stack_frame, std::optional<Value>* error_val) {
        return vm->ThrowException(stack_frame, error_val);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试ThrowException - 无异常处理表
 */
TEST_F(VMExceptionTest, ThrowException_NoExceptionTable) {
    // Arrange
    VM vm(context_.get());
    stack_frame_->set_function_def(function_def_.get());
    Value error_val = Error::Throw(context_.get(), "Test error");
    std::optional<Value> error_opt = error_val;

    // Act
    bool handled = ThrowException(&vm, stack_frame_.get(), &error_opt);

    // Assert
    EXPECT_FALSE(handled);
}

/**
 * @test 测试ThrowException - 有Catch块
 */
TEST_F(VMExceptionTest, ThrowException_WithCatch) {
    // Arrange
    VM vm(context_.get());
    ExceptionEntry entry;
    entry.try_start_pc = 0;
    entry.try_end_pc = 10;
    entry.catch_start_pc = 5;
    entry.catch_end_pc = 15;
    entry.catch_err_var_idx = 0;  // 设置错误变量的索引
    function_def_->exception_table().AddEntry(std::move(entry));

    stack_frame_->upgrade(1);  // 为错误变量预留空间
    stack_frame_->push(Value());  // 占位符
    stack_frame_->set_function_def(function_def_.get());
    stack_frame_->set_pc(5);  // 位于try块中

    Value error_val = Error::Throw(context_.get(), "Test error");
    std::optional<Value> error_opt = error_val;

    // Act
    bool handled = ThrowException(&vm, stack_frame_.get(), &error_opt);

    // Assert
    EXPECT_TRUE(handled);
    EXPECT_EQ(stack_frame_->pc(), 5);  // 跳转到catch块
}

// =============================================================================
// 生成器相关测试
// =============================================================================

/**
 * @class VMGeneratorTest
 * @brief VM生成器相关测试
 */
class VMGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_generator", 0);
        function_def_->set_is_generator();
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
        context_.reset();
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    void GeneratorSaveContext(VM* vm, StackFrame* stack_frame, GeneratorObject* generator) {
        vm->GeneratorSaveContext(stack_frame, generator);
    }

    void GeneratorRestoreContext(VM* vm, StackFrame* stack_frame, GeneratorObject* generator) {
        vm->GeneratorRestoreContext(stack_frame, generator);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试GeneratorSaveContext
 */
TEST_F(VMGeneratorTest, GeneratorSaveContext_SaveState) {
    // Arrange
    VM vm(context_.get());
    auto* generator = GeneratorObject::New(context_.get(), Value(function_def_.get()));
    stack_frame_->set_pc(100);
    stack_frame_->push(Value(42));
    generator->stack().resize(1);  // 调整generator的stack大小以匹配stack_frame

    // Act
    GeneratorSaveContext(&vm, stack_frame_.get(), generator);

    // Assert
    EXPECT_EQ(generator->pc(), 100);
    EXPECT_EQ(generator->stack().vector().size(), 1);
    EXPECT_EQ(generator->stack().vector()[0].i64(), 42);
}

/**
 * @test 测试GeneratorRestoreContext
 */
TEST_F(VMGeneratorTest, GeneratorRestoreContext_RestoreState) {
    // Arrange
    VM vm(context_.get());
    auto* generator = GeneratorObject::New(context_.get(), Value(function_def_.get()));
    generator->set_pc(100);
    generator->stack().push(Value(42));

    // Act
    GeneratorRestoreContext(&vm, stack_frame_.get(), generator);

    // Assert
    EXPECT_EQ(stack_frame_->pc(), 100);
    EXPECT_TRUE(generator->IsExecuting());
}

// =============================================================================
// 集成测试
// =============================================================================

/**
 * @class VMIntegrationTest
 * @brief VM集成测试
 */
class VMIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        context_ = std::make_unique<Context>(runtime_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
    }

    void TearDown() override {
        context_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    // 辅助函数：添加常量到常量池
    ConstIndex AddConstant(const Value& value) {
        return context_->FindConstOrInsertToGlobal(value);
    }

    // 辅助函数：根据常量索引发出加载常量指令
    void EmitLoadConst(BytecodeTable& bytecode_table, ConstIndex const_idx) {
        if (const_idx <= 5) {
            bytecode_table.EmitOpcode(OpcodeType::kCLoad_0 + const_idx);
        } else {
            bytecode_table.EmitOpcode(OpcodeType::kCLoadD);
            bytecode_table.EmitU32(const_idx);
        }
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::shared_ptr<ModuleDef> module_def_;
};

/**
 * @test 测试简单的函数调用集成
 */
TEST_F(VMIntegrationTest, SimpleFunctionCall) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "test", 0);

    Value const_val(42);
    ConstIndex const_idx = AddConstant(const_val);
    EmitLoadConst(func->bytecode_table(), const_idx);
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 42);
}

/**
 * @test 测试算术运算集成
 */
TEST_F(VMIntegrationTest, ArithmeticOperations) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "calc", 0);

    // 计算 (10 + 20) * 2 - 5 = 35
    auto idx10 = AddConstant(Value(10));
    auto idx20 = AddConstant(Value(20));
    auto idx2 = AddConstant(Value(2));
    auto idx5 = AddConstant(Value(5));

    EmitLoadConst(func->bytecode_table(), idx10);  // 10
    EmitLoadConst(func->bytecode_table(), idx20);  // 20
    func->bytecode_table().EmitOpcode(OpcodeType::kAdd);      // 30
    EmitLoadConst(func->bytecode_table(), idx2);  // 2
    func->bytecode_table().EmitOpcode(OpcodeType::kMul);      // 60
    EmitLoadConst(func->bytecode_table(), idx5);  // 5
    func->bytecode_table().EmitOpcode(OpcodeType::kSub);      // 55
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 55);
}

/**
 * @test 测试条件跳转集成
 */
TEST_F(VMIntegrationTest, ConditionalJump) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "conditional", 0);

    // 创建一个新的 runtime 和 context 来确保常量池是干净的
    auto clean_runtime = TestRuntime::Create();
    auto clean_context = std::make_unique<Context>(clean_runtime.get());

    // 添加常量到干净的 context
    auto idx_true = clean_context->FindConstOrInsertToGlobal(Value(true));
    auto idx_42 = clean_context->FindConstOrInsertToGlobal(Value(42));
    auto idx_0 = clean_context->FindConstOrInsertToGlobal(Value(0));

    // if (true) { return 42; } else { return 0; }
    func->bytecode_table().EmitConstLoad(idx_true);  // 加载 true
    func->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
    func->bytecode_table().EmitI16(3);  // 跳转到 else 分支（假设都是单字节指令）
    func->bytecode_table().EmitConstLoad(idx_42);  // then分支: 42
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);
    func->bytecode_table().EmitConstLoad(idx_0);  // else分支: 0
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = clean_context->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 42);
}

/**
 * @test 测试比较运算集成
 */
TEST_F(VMIntegrationTest, ComparisonOperations) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "compare", 0);

    // 测试 10 < 20
    func->bytecode_table().EmitOpcode(OpcodeType::kCLoad_0);  // 10
    func->bytecode_table().EmitOpcode(OpcodeType::kCLoad_1);  // 20
    func->bytecode_table().EmitOpcode(OpcodeType::kLt);
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    context_->FindConstOrInsertToGlobal(Value(10));
    context_->FindConstOrInsertToGlobal(Value(20));

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_TRUE(result.ToBoolean().boolean());
}

/**
 * @test 测试位运算集成
 */
TEST_F(VMIntegrationTest, BitwiseOperations) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "bitwise", 0);

    // 创建一个新的 runtime 和 context 来确保常量池是干净的
    auto clean_runtime = TestRuntime::Create();
    auto clean_context = std::make_unique<Context>(clean_runtime.get());

    // 测试 (5 & 3) | 2 = 1 | 2 = 3
    auto idx_5 = clean_context->FindConstOrInsertToGlobal(Value(5));
    auto idx_3 = clean_context->FindConstOrInsertToGlobal(Value(3));
    auto idx_2 = clean_context->FindConstOrInsertToGlobal(Value(2));

    func->bytecode_table().EmitConstLoad(idx_5);  // 5
    func->bytecode_table().EmitConstLoad(idx_3);  // 3
    func->bytecode_table().EmitOpcode(OpcodeType::kBitAnd);   // 1
    func->bytecode_table().EmitConstLoad(idx_2);  // 2
    func->bytecode_table().EmitOpcode(OpcodeType::kBitOr);    // 3
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = clean_context->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 3);
}

/**
 * @test 测试递增/递减运算
 */
TEST_F(VMIntegrationTest, IncrementDecrementOperations) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "inc", 1);

    // 测试参数递增
    func->bytecode_table().EmitOpcode(OpcodeType::kVLoad_0);
    func->bytecode_table().EmitOpcode(OpcodeType::kInc);
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args = {Value(10)};

    // Act
    auto result = context_->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 11);
}

/**
 * @test 测试取负运算
 */
TEST_F(VMIntegrationTest, NegationOperation) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "neg", 0);

    // 创建一个新的 runtime 和 context 来确保常量池是干净的
    auto clean_runtime = TestRuntime::Create();
    auto clean_context = std::make_unique<Context>(clean_runtime.get());

    // 测试 -42
    auto idx_42 = clean_context->FindConstOrInsertToGlobal(Value(42));

    func->bytecode_table().EmitConstLoad(idx_42);  // 42
    func->bytecode_table().EmitOpcode(OpcodeType::kNeg);
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = clean_context->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), -42);
}

/**
 * @test 测试移位运算
 */
TEST_F(VMIntegrationTest, ShiftOperations) {
    // Arrange
    auto* func = TestFunctionDef::Create(module_def_.get(), "shift", 0);

    // 创建一个新的 runtime 和 context 来确保常量池是干净的
    auto clean_runtime = TestRuntime::Create();
    auto clean_context = std::make_unique<Context>(clean_runtime.get());

    // 测试 (8 << 2) >> 1 = 32 >> 1 = 16
    auto idx_8 = clean_context->FindConstOrInsertToGlobal(Value(8));
    auto idx_2 = clean_context->FindConstOrInsertToGlobal(Value(2));
    auto idx_1 = clean_context->FindConstOrInsertToGlobal(Value(1));

    func->bytecode_table().EmitConstLoad(idx_8);  // 8
    func->bytecode_table().EmitConstLoad(idx_2);  // 2
    func->bytecode_table().EmitOpcode(OpcodeType::kShl);      // 32
    func->bytecode_table().EmitConstLoad(idx_1);  // 1
    func->bytecode_table().EmitOpcode(OpcodeType::kShr);      // 16
    func->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    Value func_val(func);
    std::vector<Value> args;

    // Act
    auto result = clean_context->CallFunction(&func_val, Value(), args.begin(), args.end());

    // Assert
    EXPECT_EQ(result.i64(), 16);
}

} // namespace test
} // namespace mjs

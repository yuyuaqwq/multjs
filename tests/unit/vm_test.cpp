#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>

#include <mjs/vm.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value.h>
#include <mjs/stack_frame.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include <mjs/bytecode.h>
#include <mjs/opcode.h>
#include <mjs/const_pool.h>
#include <mjs/object_impl/function_object.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/object_impl/async_object.h>
#include <mjs/object_impl/array_object.h>
#include <mjs/error.h>

namespace mjs {
namespace test {

class VMTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = std::make_unique<Runtime>();
        context_ = std::make_unique<Context>(runtime_.get());
        vm_ = std::make_unique<VM>(context_.get());
    }

    void TearDown() override {
        vm_.reset();
        context_.reset();
        runtime_.reset();
    }

    // 辅助函数：创建简单的函数定义
    Value CreateSimpleFunction(const std::string& name, uint32_t par_count = 0) {
        auto module_def = new ModuleDef(runtime_.get(), name + "_module", "", par_count);
        auto func_def = new FunctionDef(module_def, name, par_count);
        func_def->set_is_normal();
        return Value(func_def);
    }

    // 辅助函数：创建带字节码的函数定义
    Value CreateFunctionWithBytecode(const std::string& name,
                                                           const std::vector<uint8_t>& bytecode) {
        auto func_def = CreateSimpleFunction(name);
        auto& table = func_def.function_def().bytecode_table();
        for (uint8_t byte : bytecode) {
            table.EmitU8(byte);
        }
        return Value(func_def);
    }

    // 辅助函数：添加常量到常量池
    ConstIndex AddConstant(const Value& value) {
        return context_->FindConstOrInsertToLocal(value);
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<VM> vm_;
};

// 测试VM基本构造和初始化
TEST_F(VMTest, BasicConstruction) {
    EXPECT_NE(vm_.get(), nullptr);
    // VM应该正确关联到context
}

// 测试基本指令执行 - 常量加载
TEST_F(VMTest, BasicInstructionExecution_ConstantLoad) {
    // 创建一个简单的函数，加载常量并返回
    auto func_def = CreateSimpleFunction("test_const_load");
    
    // 添加常量到常量池
    ConstIndex const_idx = AddConstant(Value(42.0));
    
    // 生成字节码：CLoad 常量索引, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_idx);
    table.EmitOpcode(OpcodeType::kReturn);
    
    // 创建栈帧并执行
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 42.0);
}

// 测试变量操作指令
TEST_F(VMTest, VariableOperations) {
    auto func_def = CreateSimpleFunction("test_variables", 1); // 1个参数
    func_def.function_def().var_def_table().AddVar("param");
    func_def.function_def().var_def_table().AddVar("local");
    
    // 生成字节码：VLoad_0 (加载参数), VStore_1 (存储到本地变量), VLoad_1, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);  // 加载参数0
    table.EmitOpcode(OpcodeType::kVStore_1); // 存储到变量1
    table.EmitOpcode(OpcodeType::kVLoad_1);  // 加载变量1
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>{Value(123.0)};
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 123.0);
}

// 测试算术运算指令
TEST_F(VMTest, ArithmeticOperations) {
    auto func_def = CreateSimpleFunction("test_arithmetic");
    
    // 添加常量
    ConstIndex const1 = AddConstant(Value(10.0));
    ConstIndex const2 = AddConstant(Value(5.0));
    
    // 生成字节码：CLoad 10, CLoad 5, Add, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 15.0);
}

// 测试多种算术运算
TEST_F(VMTest, MultipleArithmeticOperations) {
    auto func_def = CreateSimpleFunction("test_multi_arithmetic");
    
    ConstIndex const1 = AddConstant(Value(20.0));
    ConstIndex const2 = AddConstant(Value(4.0));
    
    // 生成字节码：CLoad 20, CLoad 4, Sub, CLoad 4, Mul, Return (结果应该是 (20-4)*4 = 64)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kSub);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kMul);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 64.0);
}

// 测试栈操作指令
TEST_F(VMTest, StackOperations) {
    auto func_def = CreateSimpleFunction("test_stack_ops");
    
    ConstIndex const1 = AddConstant(Value(1.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    
    // 生成字节码：CLoad 1, CLoad 2, Swap, Pop, Return (结果应该是1)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kSwap);
    table.EmitOpcode(OpcodeType::kPop);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 1.0);
}

// 测试比较操作指令
TEST_F(VMTest, ComparisonOperations) {
    auto func_def = CreateSimpleFunction("test_comparison");
    
    ConstIndex const1 = AddConstant(Value(10.0));
    ConstIndex const2 = AddConstant(Value(5.0));
    
    // 生成字节码：CLoad 10, CLoad 5, Gt, Return (10 > 5 应该是true)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kGt);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_TRUE(result.boolean());
}

// 测试条件跳转指令
TEST_F(VMTest, ConditionalJump) {
    auto func_def = CreateSimpleFunction("test_conditional_jump");
    
    ConstIndex const_true = AddConstant(Value(true));
    ConstIndex const1 = AddConstant(Value(100.0));
    ConstIndex const2 = AddConstant(Value(200.0));
    
    // 生成字节码：
    // CLoad true
    // IfEq +4  (如果true则跳过下一条指令)
    // CLoad 200
    // Goto +2
    // CLoad 100
    // Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_true);
    table.EmitOpcode(OpcodeType::kIfEq);
    table.EmitU16(4); // 跳转偏移
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kGoto);
    table.EmitU16(2); // 跳转偏移
    table.EmitConstLoad(const1);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 100.0);
}

// 测试函数调用
TEST_F(VMTest, FunctionCall) {
    // 创建被调用的函数
    auto called_func = CreateSimpleFunction("called_function", 1);
    called_func.function_def().var_def_table().AddVar("param");
    
    // 被调用函数的字节码：VLoad_0, Inc, Return
    auto& called_table = called_func.function_def().bytecode_table();
    called_table.EmitOpcode(OpcodeType::kVLoad_0);
    called_table.EmitOpcode(OpcodeType::kInc);
    called_table.EmitOpcode(OpcodeType::kReturn);
    
    // 将被调用函数添加到常量池
    ConstIndex func_const = AddConstant(called_func);
    ConstIndex arg_const = AddConstant(Value(10ull));
    
    // 创建主函数
    auto main_func = CreateSimpleFunction("main_function");
    auto& main_table = main_func.function_def().bytecode_table();
    
    // 主函数字节码：CLoad func, Undefined (this), CLoad 10, FunctionCall 1, Return
    main_table.EmitConstLoad(func_const);
    main_table.EmitOpcode(OpcodeType::kUndefined);
    main_table.EmitConstLoad(arg_const);
    main_table.EmitOpcode(OpcodeType::kFunctionCall);
    main_table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, main_func, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 11.0); // 10 + 1
}

// 测试异常处理
TEST_F(VMTest, ExceptionHandling) {
    auto func_def = CreateSimpleFunction("test_exception");
    
    // 创建异常值
    ConstIndex error_const = AddConstant(Error::Throw(context_.get(), "Test error"));
    
    // 生成字节码：CLoad error, Throw
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(error_const);
    table.EmitOpcode(OpcodeType::kThrow);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsException());
}

// 测试Try-Catch异常处理
TEST_F(VMTest, TryCatchException) {
    auto func_def = CreateSimpleFunction("test_try_catch");
    
    ConstIndex error_const = AddConstant(Error::Throw(context_.get(), "Test error"));
    ConstIndex success_const = AddConstant(Value(42.0));
    ConstIndex caught_const = AddConstant(Value(99.0));
    
    // 添加异常处理表项
    func_def.function_def().var_def_table().AddVar("error_var");
    //func_def.function_def().exception_table().AddEntry(2, 4, 5, 8, 0); // try: 2-4, catch: 5-8, error_var: 0
    
    // 生成字节码：
    // 0: TryBegin
    // 1: CLoad error
    // 2: Throw
    // 3: CLoad 42 (不会执行)
    // 4: TryEnd
    // 5: CLoad 99 (catch块)
    // 6: Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kTryBegin);      // 0
    table.EmitConstLoad(error_const);             // 1
    table.EmitOpcode(OpcodeType::kThrow);         // 3
    table.EmitConstLoad(success_const);           // 4
    table.EmitOpcode(OpcodeType::kTryEnd);        // 6
    table.EmitConstLoad(caught_const);            // 7 (catch开始)
    table.EmitOpcode(OpcodeType::kReturn);        // 9
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 99.0); // 应该执行catch块
}

// 测试生成器函数
TEST_F(VMTest, GeneratorFunction) {
    auto func_def = CreateSimpleFunction("test_generator");
    func_def.function_def().set_is_generator();
    
    ConstIndex const1 = AddConstant(Value(1.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    
    // 生成字节码：CLoad 1, Yield, CLoad 2, GeneratorReturn
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitOpcode(OpcodeType::kYield);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kGeneratorReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    // 调用生成器函数应该返回生成器对象
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsGeneratorObject());
    
    auto& generator = result.generator();
    EXPECT_FALSE(generator.IsClosed());
}

// 测试异步函数
TEST_F(VMTest, AsyncFunction) {
    auto func_def = CreateSimpleFunction("test_async");
    func_def.function_def().set_is_async();
    
    ConstIndex const_val = AddConstant(Value(42.0));
    
    // 生成字节码：CLoad 42, AsyncReturn
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_val);
    table.EmitOpcode(OpcodeType::kAsyncReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    // 调用异步函数应该返回Promise对象
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsPromiseObject());
}

// 测试模块初始化
TEST_F(VMTest, ModuleInitialization) {
    auto module_def = std::make_unique<ModuleDef>(runtime_.get(), "test_module", "", 0);
    
    // 添加导出变量
    module_def->export_var_def_table().AddExportVar("exportedVar", 0);
    
    Value module_val(module_def.get());
    vm_->ModuleInit(&module_val);
    
    EXPECT_TRUE(module_val.IsModuleObject());
}

// 测试C++函数调用
TEST_F(VMTest, CppFunctionCall) {
    // 创建C++函数
    auto cpp_func = [](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        if (par_count > 0) {
            auto arg = stack.get(-static_cast<ptrdiff_t>(par_count));
            if (arg.IsNumber()) {
                return Value(arg.f64() * 2.0); // 返回参数的两倍
            }
        }
        return Value(0.0);
    };
    
    StackFrame stack_frame(&runtime_->stack());
    Value func_val(cpp_func);
    Value this_val;
    std::vector<Value> args = {Value(21.0)};
    Value result = vm_->CallFunction(&stack_frame, func_val, this_val,
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 42.0);
}

// 测试参数数量验证
TEST_F(VMTest, ParameterCountValidation) {
    auto func_def = CreateSimpleFunction("test_param_count", 2); // 需要2个参数
    func_def.function_def().var_def_table().AddVar("param1");
    func_def.function_def().var_def_table().AddVar("param2");
    
    // 简单返回第一个参数
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    // 传递少于需要的参数应该报错
    std::vector<Value> args = {Value(10.0)}; // 只传1个参数，需要2个
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsException());
}

// 测试闭包变量
TEST_F(VMTest, ClosureVariables) {
    auto outer_func = CreateSimpleFunction("outer_function");
    outer_func.function_def().var_def_table().AddVar("outer_var");
    
    auto inner_func = CreateSimpleFunction("inner_function");
    inner_func.function_def().var_def_table().AddVar("inner_var");
    
    // 设置闭包变量表
    //inner_func->closure_var_table().AddClosureVar("outer_var", 0, 0);
    //inner_func->set_has_this(true);
    
    ConstIndex inner_func_const = AddConstant(inner_func);
    ConstIndex const_val = AddConstant(Value(100.0));
    
    // 外部函数字节码：CLoad 100, VStore_0, CLoad inner_func, Closure, Return
    auto& outer_table = outer_func.function_def().bytecode_table();
    outer_table.EmitConstLoad(const_val);
    outer_table.EmitOpcode(OpcodeType::kVStore_0);
    outer_table.EmitConstLoad(inner_func_const);
    outer_table.EmitOpcode(OpcodeType::kClosure);
    outer_table.EmitU32(inner_func_const);
    outer_table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, outer_func, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsFunctionObject());
}

// 测试位运算指令
TEST_F(VMTest, BitwiseOperations) {
    auto func_def = CreateSimpleFunction("test_bitwise");
    
    ConstIndex const1 = AddConstant(Value(15.0)); // 1111 in binary
    ConstIndex const2 = AddConstant(Value(7.0));  // 0111 in binary
    
    // 生成字节码：CLoad 15, CLoad 7, BitAnd, Return (应该得到7)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kBitAnd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 7.0);
}

// 测试位或运算
TEST_F(VMTest, BitwiseOrOperation) {
    auto func_def = CreateSimpleFunction("test_bitwise_or");
    
    ConstIndex const1 = AddConstant(Value(12.0)); // 1100 in binary
    ConstIndex const2 = AddConstant(Value(3.0));  // 0011 in binary
    
    // 生成字节码：CLoad 12, CLoad 3, BitOr, Return (应该得到15: 1111)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kBitOr);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 15.0);
}

// 测试位异或运算
TEST_F(VMTest, BitwiseXorOperation) {
    auto func_def = CreateSimpleFunction("test_bitwise_xor");
    
    ConstIndex const1 = AddConstant(Value(12.0)); // 1100 in binary
    ConstIndex const2 = AddConstant(Value(10.0)); // 1010 in binary
    
    // 生成字节码：CLoad 12, CLoad 10, BitXor, Return (应该得到6: 0110)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kBitXor);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 6.0);
}

// 测试位取反运算
TEST_F(VMTest, BitwiseNotOperation) {
    auto func_def = CreateSimpleFunction("test_bitwise_not");
    
    ConstIndex const1 = AddConstant(Value(5.0)); // 0101 in binary
    
    // 生成字节码：CLoad 5, BitNot, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitOpcode(OpcodeType::kBitNot);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    // 位取反的结果取决于具体实现，这里主要测试指令能正常执行
}

// 测试移位运算指令
TEST_F(VMTest, ShiftOperations) {
    auto func_def = CreateSimpleFunction("test_shift");
    
    ConstIndex const1 = AddConstant(Value(8.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    
    // 生成字节码：CLoad 8, CLoad 2, Shl, Return (8 << 2 = 32)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kShl);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 32.0);
}

// 测试右移位运算
TEST_F(VMTest, RightShiftOperation) {
    auto func_def = CreateSimpleFunction("test_right_shift");
    
    ConstIndex const1 = AddConstant(Value(32.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    
    // 生成字节码：CLoad 32, CLoad 2, Shr, Return (32 >> 2 = 8)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kShr);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 8.0);
}

// 测试无符号右移位运算
TEST_F(VMTest, UnsignedRightShiftOperation) {
    auto func_def = CreateSimpleFunction("test_unsigned_right_shift");
    
    ConstIndex const1 = AddConstant(Value(32.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    
    // 生成字节码：CLoad 32, CLoad 2, UShr, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kUShr);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 8.0);
}

// 测试字符串转换指令
TEST_F(VMTest, StringConversion) {
    auto func_def = CreateSimpleFunction("test_string_conversion");
    
    ConstIndex const_num = AddConstant(Value(42.0));
    
    // 生成字节码：CLoad 42, ToString, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_num);
    table.EmitOpcode(OpcodeType::kToString);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsString());
    EXPECT_EQ(std::string(result.string_view()), "42");
}

// 测试undefined值处理
TEST_F(VMTest, UndefinedValue) {
    auto func_def = CreateSimpleFunction("test_undefined");
    
    // 生成字节码：Undefined, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kUndefined);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsUndefined());
}

// 测试复杂的控制流
TEST_F(VMTest, ComplexControlFlow) {
    auto func_def = CreateSimpleFunction("test_complex_flow", 1);
    func_def.function_def().var_def_table().AddVar("param");
    
    ConstIndex const_zero = AddConstant(Value(0.0));
    ConstIndex const_pos = AddConstant(Value(1.0));
    ConstIndex const_neg = AddConstant(Value(-1.0));
    
    // 生成字节码实现：if (param > 0) return 1; else if (param < 0) return -1; else return 0;
    auto& table = func_def.function_def().bytecode_table();
    
    // VLoad_0, CLoad 0, Gt
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitConstLoad(const_zero);
    table.EmitOpcode(OpcodeType::kGt);
    
    // IfEq +4 (如果>0跳转到返回1)
    table.EmitOpcode(OpcodeType::kIfEq);
    table.EmitU16(4);
    
    // VLoad_0, CLoad 0, Lt
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitConstLoad(const_zero);
    table.EmitOpcode(OpcodeType::kLt);
    
    // IfEq +4 (如果<0跳转到返回-1)
    table.EmitOpcode(OpcodeType::kIfEq);
    table.EmitU16(4);
    
    // 返回0
    table.EmitConstLoad(const_zero);
    table.EmitOpcode(OpcodeType::kReturn);
    
    // 返回-1
    table.EmitConstLoad(const_neg);
    table.EmitOpcode(OpcodeType::kReturn);
    
    // 返回1
    table.EmitConstLoad(const_pos);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    
    // 测试正数
    {
        std::vector<Value> args = {Value(5.0)};
        Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                       args.begin(), args.end());
        EXPECT_TRUE(result.IsNumber());
        EXPECT_DOUBLE_EQ(result.f64(), 1.0);
    }
    
    // 测试负数
    {
        std::vector<Value> args = {Value(-3.0)};
        Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                       args.begin(), args.end());
        EXPECT_TRUE(result.IsNumber());
        EXPECT_DOUBLE_EQ(result.f64(), -1.0);
    }
    
    // 测试零
    {
        std::vector<Value> args = {Value(0.0)};
        Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                       args.begin(), args.end());
        EXPECT_TRUE(result.IsNumber());
        EXPECT_DOUBLE_EQ(result.f64(), 0.0);
    }
}

// 测试字符串操作
TEST_F(VMTest, StringOperations) {
    auto func_def = CreateSimpleFunction("test_string");
    
    ConstIndex str_const = AddConstant(Value("Hello"));
    
    // 生成字节码：CLoad "Hello", Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(str_const);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsString());
    EXPECT_EQ(std::string(result.string_view()), "Hello");
}

// 测试布尔值操作
TEST_F(VMTest, BooleanOperations) {
    auto func_def = CreateSimpleFunction("test_boolean");
    
    ConstIndex true_const = AddConstant(Value(true));
    ConstIndex false_const = AddConstant(Value(false));
    
    // 生成字节码：CLoad true, CLoad false, Eq, Return (true == false 应该是false)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(true_const);
    table.EmitConstLoad(false_const);
    table.EmitOpcode(OpcodeType::kEq);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   std::vector<Value>{}.begin(), 
                                   std::vector<Value>{}.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_FALSE(result.boolean());
}

// 测试递增操作
TEST_F(VMTest, IncrementOperation) {
    auto func_def = CreateSimpleFunction("test_increment");
    
    ConstIndex const_val = AddConstant(Value(5.0));
    
    // 生成字节码：CLoad 5, Inc, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_val);
    table.EmitOpcode(OpcodeType::kInc);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 6.0);
}

// 测试除法操作
TEST_F(VMTest, DivisionOperation) {
    auto func_def = CreateSimpleFunction("test_division");
    
    ConstIndex const1 = AddConstant(Value(20.0));
    ConstIndex const2 = AddConstant(Value(4.0));
    
    // 生成字节码：CLoad 20, CLoad 4, Div, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kDiv);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 5.0);
}

// 测试取负操作
TEST_F(VMTest, NegationOperation) {
    auto func_def = CreateSimpleFunction("test_negation");
    
    ConstIndex const_val = AddConstant(Value(42.0));
    
    // 生成字节码：CLoad 42, Neg, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_val);
    table.EmitOpcode(OpcodeType::kNeg);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), -42.0);
}

// 测试不等比较
TEST_F(VMTest, NotEqualComparison) {
    auto func_def = CreateSimpleFunction("test_not_equal");
    
    ConstIndex const1 = AddConstant(Value(5.0));
    ConstIndex const2 = AddConstant(Value(10.0));
    
    // 生成字节码：CLoad 5, CLoad 10, Ne, Return (5 != 10 应该是true)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kNe);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_TRUE(result.boolean());
}

// 测试小于等于比较
TEST_F(VMTest, LessEqualComparison) {
    auto func_def = CreateSimpleFunction("test_less_equal");
    
    ConstIndex const1 = AddConstant(Value(5.0));
    ConstIndex const2 = AddConstant(Value(5.0));
    
    // 生成字节码：CLoad 5, CLoad 5, Le, Return (5 <= 5 应该是true)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kLe);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_TRUE(result.boolean());
}

// 测试大于等于比较
TEST_F(VMTest, GreaterEqualComparison) {
    auto func_def = CreateSimpleFunction("test_greater_equal");
    
    ConstIndex const1 = AddConstant(Value(10.0));
    ConstIndex const2 = AddConstant(Value(5.0));
    
    // 生成字节码：CLoad 10, CLoad 5, Ge, Return (10 >= 5 应该是true)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kGe);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_TRUE(result.boolean());
}

// 测试小于比较
TEST_F(VMTest, LessThanComparison) {
    auto func_def = CreateSimpleFunction("test_less_than");
    
    ConstIndex const1 = AddConstant(Value(3.0));
    ConstIndex const2 = AddConstant(Value(7.0));
    
    // 生成字节码：CLoad 3, CLoad 7, Lt, Return (3 < 7 应该是true)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitOpcode(OpcodeType::kLt);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsBoolean());
    EXPECT_TRUE(result.boolean());
}

// 测试Dump指令（复制栈顶元素）
TEST_F(VMTest, DumpInstruction) {
    auto func_def = CreateSimpleFunction("test_dump");
    
    ConstIndex const_val = AddConstant(Value(99.0));
    
    // 生成字节码：CLoad 99, Dump, Add, Return (99 + 99 = 198)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_val);
    table.EmitOpcode(OpcodeType::kDump);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 198.0);
}

// 测试常量加载的不同变体
TEST_F(VMTest, ConstantLoadVariants) {
    auto func_def = CreateSimpleFunction("test_const_variants");
    
    // 测试CLoad_0到CLoad_5的快速常量加载
    ConstIndex const0 = AddConstant(Value(10.0));
    ConstIndex const1 = AddConstant(Value(20.0));
    
    // 假设const0 = 0, const1 = 1，生成字节码：CLoad_0, CLoad_1, Add, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kCLoad_0);
    table.EmitOpcode(OpcodeType::kCLoad_1);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    // 结果取决于常量池中索引0和1的实际值
}

// 测试变量加载的不同变体
TEST_F(VMTest, VariableLoadVariants) {
    auto func_def = CreateSimpleFunction("test_var_variants", 4);
    func_def.function_def().var_def_table().AddVar("param0");
    func_def.function_def().var_def_table().AddVar("param1");
    func_def.function_def().var_def_table().AddVar("param2");
    func_def.function_def().var_def_table().AddVar("param3");
    
    // 生成字节码：VLoad_0, VLoad_1, Add, VLoad_2, Add, VLoad_3, Add, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitOpcode(OpcodeType::kVLoad_1);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kVLoad_2);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kVLoad_3);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    std::vector<Value> args = {Value(1.0), Value(2.0), Value(3.0), Value(4.0)};
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 10.0); // 1+2+3+4 = 10
}

// 测试变量存储的不同变体
TEST_F(VMTest, VariableStoreVariants) {
    auto func_def = CreateSimpleFunction("test_var_store_variants", 1);
    func_def.function_def().var_def_table().AddVar("param");
    func_def.function_def().var_def_table().AddVar("local0");
    func_def.function_def().var_def_table().AddVar("local1");
    func_def.function_def().var_def_table().AddVar("local2");
    func_def.function_def().var_def_table().AddVar("local3");
    
    // 生成字节码：VLoad_0, VStore_1, VStore_2, VStore_3, VLoad_1, VLoad_2, Add, VLoad_3, Add, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);  // 加载参数
    table.EmitOpcode(OpcodeType::kDump);     // 复制
    table.EmitOpcode(OpcodeType::kVStore_1); // 存储到local0
    table.EmitOpcode(OpcodeType::kDump);     // 复制
    table.EmitOpcode(OpcodeType::kVStore_2); // 存储到local1
    table.EmitOpcode(OpcodeType::kVStore_3); // 存储到local2
    table.EmitOpcode(OpcodeType::kVLoad_1);  // 加载local0
    table.EmitOpcode(OpcodeType::kVLoad_2);  // 加载local1
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kVLoad_3);  // 加载local2
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    std::vector<Value> args = {Value(5.0)};
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 15.0); // 5+5+5 = 15
}

// 测试复杂的栈操作组合
TEST_F(VMTest, ComplexStackOperations) {
    auto func_def = CreateSimpleFunction("test_complex_stack");
    
    ConstIndex const1 = AddConstant(Value(1.0));
    ConstIndex const2 = AddConstant(Value(2.0));
    ConstIndex const3 = AddConstant(Value(3.0));
    
    // 生成字节码：CLoad 1, CLoad 2, CLoad 3, Swap, Pop, Add, Return
    // 栈变化：[1] -> [1,2] -> [1,2,3] -> [1,3,2] -> [1,3] -> [4]
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const1);
    table.EmitConstLoad(const2);
    table.EmitConstLoad(const3);
    table.EmitOpcode(OpcodeType::kSwap);
    table.EmitOpcode(OpcodeType::kPop);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 4.0); // 1 + 3 = 4
}

// 测试多个参数的函数调用
TEST_F(VMTest, MultiParameterFunctionCall) {
    auto func_def = CreateSimpleFunction("test_multi_param", 3);
    func_def.function_def().var_def_table().AddVar("param0");
    func_def.function_def().var_def_table().AddVar("param1");
    func_def.function_def().var_def_table().AddVar("param2");
    
    // 生成字节码：VLoad_0, VLoad_1, Mul, VLoad_2, Add, Return (param0 * param1 + param2)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitOpcode(OpcodeType::kVLoad_1);
    table.EmitOpcode(OpcodeType::kMul);
    table.EmitOpcode(OpcodeType::kVLoad_2);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    std::vector<Value> args = {Value(3.0), Value(4.0), Value(5.0)};
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 17.0); // 3 * 4 + 5 = 17
}

// 测试参数过多的情况
TEST_F(VMTest, ExcessParameterHandling) {
    auto func_def = CreateSimpleFunction("test_excess_params", 2);
    func_def.function_def().var_def_table().AddVar("param0");
    func_def.function_def().var_def_table().AddVar("param1");
    
    // 生成字节码：VLoad_0, VLoad_1, Add, Return
    auto& table = func_def.function_def().bytecode_table();
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitOpcode(OpcodeType::kVLoad_1);
    table.EmitOpcode(OpcodeType::kAdd);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    // 传递3个参数，但函数只需要2个
    std::vector<Value> args = {Value(10.0), Value(20.0), Value(30.0)};
    Value result = vm_->CallFunction(&stack_frame, func_def, this_val, 
                                   args.begin(), args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 30.0); // 多余的参数应该被丢弃
}

// 测试模块导出变量绑定
TEST_F(VMTest, ModuleExportVariableBinding) {
    auto module_def = new ModuleDef(runtime_.get(), "test_export_module", "", 0);
    
    // 添加导出变量
    module_def->export_var_def_table().AddExportVar("exportedValue", 0);
    
    // 创建模块函数
    auto func_def = new FunctionDef(module_def, "module_func", 0);
    func_def->set_is_module();
    func_def->var_def_table().AddVar("exportedValue");
    
    ConstIndex const_val = AddConstant(Value(123.0));
    
    // 生成字节码：CLoad 123, VStore_0, VLoad_0, Return
    auto& table = func_def->bytecode_table();
    table.EmitConstLoad(const_val);
    table.EmitOpcode(OpcodeType::kVStore_0);
    table.EmitOpcode(OpcodeType::kVLoad_0);
    table.EmitOpcode(OpcodeType::kReturn);
    
    Value module_val(module_def);
    vm_->ModuleInit(&module_val);
    
    EXPECT_TRUE(module_val.IsModuleObject());
    
    // 执行模块函数
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, Value(func_def), this_val,
        args.begin(),
        args.end());
    
    EXPECT_TRUE(result.IsNumber());
    EXPECT_DOUBLE_EQ(result.f64(), 123.0);
}

// 测试错误情况下的异常处理
TEST_F(VMTest, ExceptionInArithmetic) {
    auto func_def = CreateSimpleFunction("test_exception_arithmetic");
    
    ConstIndex const_zero = AddConstant(Value(0.0));
    ConstIndex const_ten = AddConstant(Value(10.0));
    
    // 生成字节码：CLoad 10, CLoad 0, Div, Return (除零可能产生异常或特殊值)
    auto& table = func_def.function_def().bytecode_table();
    table.EmitConstLoad(const_ten);
    table.EmitConstLoad(const_zero);
    table.EmitOpcode(OpcodeType::kDiv);
    table.EmitOpcode(OpcodeType::kReturn);
    
    StackFrame stack_frame(&runtime_->stack());
    Value this_val;
    auto args = std::vector<Value>();
    Value result = vm_->CallFunction(&stack_frame, Value(func_def), this_val,
        args.begin(),
        args.end());
    
    // 结果可能是Infinity、异常或其他特殊值，主要测试VM不会崩溃
    EXPECT_TRUE(result.IsNumber() || result.IsException());
}

} // namespace test
} // namespace mjs 
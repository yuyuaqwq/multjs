/**
 * @file bytecode_test.cpp
 * @brief 字节码表系统单元测试
 *
 * 测试BytecodeTable的功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>

#include <mjs/bytecode_table.h>
#include <mjs/opcode.h>
#include <mjs/runtime.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include "test_helpers.h"

namespace mjs {
namespace test {

/**
 * @class BytecodeTableTest
 * @brief 字节码表基础测试
 */
class BytecodeTableTest : public ::testing::Test {
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
 * @test 测试字节码表初始大小
 */
TEST_F(BytecodeTableTest, BytecodeTableInitialSize) {
    // Arrange & Act
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Assert
    EXPECT_EQ(bytecode_table.size(), 0);
}

/**
 * @test 测试发射操作码
 */
TEST_F(BytecodeTableTest, EmitOpcode) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitOpcode(OpcodeType::kNop);

    // Assert
    EXPECT_EQ(bytecode_table.size(), 1);
    EXPECT_EQ(bytecode_table.GetOpcode(0), OpcodeType::kNop);
}

/**
 * @test 测试发射多个操作码
 */
TEST_F(BytecodeTableTest, EmitMultipleOpcodes) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitOpcode(OpcodeType::kNop);
    bytecode_table.EmitOpcode(OpcodeType::kLdNull);
    bytecode_table.EmitOpcode(OpcodeType::kLdUndef);

    // Assert
    EXPECT_EQ(bytecode_table.size(), 3);
    EXPECT_EQ(bytecode_table.GetOpcode(0), OpcodeType::kNop);
    EXPECT_EQ(bytecode_table.GetOpcode(1), OpcodeType::kLdNull);
    EXPECT_EQ(bytecode_table.GetOpcode(2), OpcodeType::kLdUndef);
}

/**
 * @test 测试发射PC偏移
 */
TEST_F(BytecodeTableTest, EmitPcOffset) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitPcOffset(100);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射变量索引
 */
TEST_F(BytecodeTableTest, EmitVarIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitVarIndex(5);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射常量索引
 */
TEST_F(BytecodeTableTest, EmitConstIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitConstIndex(10);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射常量加载指令(小索引)
 */
TEST_F(BytecodeTableTest, EmitConstLoadSmallIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act - 索引0-5应该使用专用指令
    bytecode_table.EmitConstLoad(0);
    bytecode_table.EmitConstLoad(3);
    bytecode_table.EmitConstLoad(5);

    // Assert
    EXPECT_GE(bytecode_table.size(), 3);
}

/**
 * @test 测试发射常量加载指令(大索引)
 */
TEST_F(BytecodeTableTest, EmitConstLoadLargeIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act - 大索引应该使用通用指令
    bytecode_table.EmitConstLoad(100);
    bytecode_table.EmitConstLoad(1000);

    // Assert
    EXPECT_GE(bytecode_table.size(), 2);
}

/**
 * @test 测试发射变量存储指令
 */
TEST_F(BytecodeTableTest, EmitVarStore) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitVarStore(3);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射变量加载指令
 */
TEST_F(BytecodeTableTest, EmitVarLoad) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitVarLoad(2);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射跳转指令
 */
TEST_F(BytecodeTableTest, EmitGoto) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitGoto();

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试发射属性加载指令
 */
TEST_F(BytecodeTableTest, EmitPropertyLoad) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitPropertyLoad(5);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试获取变量索引
 */
TEST_F(BytecodeTableTest, GetVarIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();
    bytecode_table.EmitVarIndex(42);

    // Act
    Pc pc = 0;
    VarIndex var_index = bytecode_table.GetVarIndex(&pc);

    // Assert
    EXPECT_EQ(var_index, 42);
}

/**
 * @test 测试获取常量索引
 */
TEST_F(BytecodeTableTest, GetConstIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();
    bytecode_table.EmitConstIndex(99);

    // Act
    Pc pc = 0;
    ConstIndex const_index = bytecode_table.GetConstIndex(&pc);

    // Assert
    EXPECT_EQ(const_index, 99);
}

/**
 * @test 测试获取PC
 */
TEST_F(BytecodeTableTest, GetPc) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();
    bytecode_table.EmitOpcode(OpcodeType::kNop);
    bytecode_table.EmitOpcode(OpcodeType::kLdNull);

    // Act
    Pc pc = 0;
    Pc retrieved_pc = bytecode_table.GetPc(&pc);

    // Assert
    EXPECT_EQ(retrieved_pc, pc);
}

/**
 * @class BytecodeTableComplexTest
 * @brief 字节码表复杂测试
 */
class BytecodeTableComplexTest : public ::testing::Test {
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
 * @test 测试混合指令序列
 */
TEST_F(BytecodeTableComplexTest, MixedInstructionSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act - 发射一个混合指令序列
    bytecode_table.EmitOpcode(OpcodeType::kNop);
    bytecode_table.EmitConstLoad(0);
    bytecode_table.EmitVarLoad(1);
    bytecode_table.EmitOpcode(OpcodeType::kAdd);
    bytecode_table.EmitVarStore(2);

    // Assert
    EXPECT_GT(bytecode_table.size(), 5);
}

/**
 * @test 测试条件跳转指令序列
 */
TEST_F(BytecodeTableComplexTest, ConditionalJumpSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitConstLoad(0);
    bytecode_table.EmitVarLoad(1);
    bytecode_table.EmitOpcode(OpcodeType::kGt);
    bytecode_table.EmitGoto();

    // Assert
    EXPECT_GT(bytecode_table.size(), 4);
}

/**
 * @test 测试函数调用指令序列
 */
TEST_F(BytecodeTableComplexTest, FunctionCallSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitConstLoad(0);  // 加载参数
    bytecode_table.EmitConstLoad(1);
    bytecode_table.EmitConstLoad(2);
    bytecode_table.EmitOpcode(OpcodeType::kCall);  // 调用函数

    // Assert
    EXPECT_GT(bytecode_table.size(), 4);
}

/**
 * @test 测试对象创建指令序列
 */
TEST_F(BytecodeTableComplexTest, ObjectCreationSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitOpcode(OpcodeType::kNewObj);
    bytecode_table.EmitConstLoad(0);  // 属性名
    bytecode_table.EmitConstLoad(1);  // 属性值
    bytecode_table.EmitOpcode(OpcodeType::kSetProperty);

    // Assert
    EXPECT_GT(bytecode_table.size(), 4);
}

/**
 * @test 测试数组创建指令序列
 */
TEST_F(BytecodeTableComplexTest, ArrayCreationSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitOpcode(OpcodeType::kNewArr);
    bytecode_table.EmitConstLoad(0);
    bytecode_table.EmitConstLoad(1);
    bytecode_table.EmitConstLoad(2);
    bytecode_table.EmitOpcode(OpcodeType::kSetElem);

    // Assert
    EXPECT_GT(bytecode_table.size(), 5);
}

/**
 * @test 测试返回指令序列
 */
TEST_F(BytecodeTableComplexTest, ReturnSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    bytecode_table.EmitConstLoad(0);
    bytecode_table.EmitOpcode(OpcodeType::kReturn);

    // Assert
    EXPECT_GT(bytecode_table.size(), 2);
}

/**
 * @class BytecodeTableEdgeCaseTest
 * @brief 字节码表边缘情况测试
 */
class BytecodeTableEdgeCaseTest : public ::testing::Test {
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
 * @test 测试最大变量索引
 */
TEST_F(BytecodeTableEdgeCaseTest, MaximumVarIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    VarIndex max_index = 65535; // 假设最大值
    bytecode_table.EmitVarIndex(max_index);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试最大常量索引
 */
TEST_F(BytecodeTableEdgeCaseTest, MaximumConstIndex) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    ConstIndex max_index = 2147483647; // 假设最大值
    bytecode_table.EmitConstIndex(max_index);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试最大PC偏移
 */
TEST_F(BytecodeTableEdgeCaseTest, MaximumPcOffset) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    PcOffset max_offset = 2147483647; // 假设最大值
    bytecode_table.EmitPcOffset(max_offset);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试负数PC偏移(向后跳转)
 */
TEST_F(BytecodeTableEdgeCaseTest, NegativePcOffset) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    PcOffset negative_offset = -10;
    bytecode_table.EmitPcOffset(negative_offset);

    // Assert
    EXPECT_GT(bytecode_table.size(), 0);
}

/**
 * @test 测试空指令序列
 */
TEST_F(BytecodeTableEdgeCaseTest, EmptyInstructionSequence) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act & Assert
    EXPECT_EQ(bytecode_table.size(), 0);
}

/**
 * @test 测试连续相同指令
 */
TEST_F(BytecodeTableEdgeCaseTest, ConsecutiveSameInstructions) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act
    for (int i = 0; i < 100; i++) {
        bytecode_table.EmitOpcode(OpcodeType::kNop);
    }

    // Assert
    EXPECT_EQ(bytecode_table.size(), 100);
}

/**
 * @test 测试非常量索引边界值
 */
TEST_F(BytecodeTableEdgeCaseTest, ConstIndexBoundaryValues) {
    // Arrange
    auto& bytecode_table = env_->function_def()->bytecode_table();

    // Act - 测试边界值
    bytecode_table.EmitConstLoad(0);   // 最小专用指令
    bytecode_table.EmitConstLoad(5);   // 最大专用指令
    bytecode_table.EmitConstLoad(127); // 单字节边界
    bytecode_table.EmitConstLoad(128); // 双字节起始
    bytecode_table.EmitConstLoad(32767); // 双字节边界
    bytecode_table.EmitConstLoad(32768); // 四字节起始

    // Assert
    EXPECT_GE(bytecode_table.size(), 6);
}

/**
 * @class BytecodeTableIntegrationTest
 * @brief 字节码表集成测试
 */
class BytecodeTableIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 0);
    }

    void TearDown() override {
        function_def_.reset();
        module_def_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试函数反汇编
 */
TEST_F(BytecodeTableIntegrationTest, FunctionDisassembly) {
    // Arrange
    Context context(runtime_.get());
    auto& bytecode_table = function_def_->bytecode_table();
    bytecode_table.EmitOpcode(OpcodeType::kNop);
    bytecode_table.EmitConstLoad(0);

    // Act
    std::string disassembly = function_def_->Disassembly(&context);

    // Assert
    EXPECT_TRUE(!disassembly.empty());
}

/**
 * @test 测试多个函数的字节码独立性
 */
TEST_F(BytecodeTableIntegrationTest, MultipleFunctionsIndependence) {
    // Arrange
    auto* func1 = TestFunctionDef::Create(module_def_.get(), "func1", 0);
    auto* func2 = TestFunctionDef::Create(module_def_.get(), "func2", 0);

    // Act
    func1->bytecode_table().EmitOpcode(OpcodeType::kNop);
    func2->bytecode_table().EmitOpcode(OpcodeType::kLdNull);

    // Assert - 两个函数的字节码表应该独立
    EXPECT_EQ(func1->bytecode_table().GetOpcode(0), OpcodeType::kNop);
    EXPECT_EQ(func2->bytecode_table().GetOpcode(0), OpcodeType::kLdNull);
}

} // namespace test
} // namespace mjs

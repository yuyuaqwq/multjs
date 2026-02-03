/**
 * @file jump_manager_test.cpp
 * @brief JumpManager单元测试
 *
 * 测试JumpManager类的功能,包括:
 * - break语句的跳转指令修复
 * - continue语句的跳转指令修复
 * - 标签管理
 * - 循环跳转处理
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/value/function_def.h>

#include "src/compiler/jump_manager.h"
#include "src/compiler/repair_def.h"
#include "tests/unit/test_helpers.h"

namespace mjs {

class JumpManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试环境
        env_ = std::make_unique<test::TestEnvironment>();
        function_def_ = env_->function_def();

        // 获取字节码表
        bytecode_table_ = &function_def_->bytecode_table();
    }

    void TearDown() override {
        // 清理资源
        bytecode_table_ = nullptr;
        function_def_ = nullptr;
        env_.reset();
    }

    /**
     * @brief 辅助方法:发射一条跳转指令
     * @return 跳转指令的PC位置
     */
    Pc EmitJumpInstruction() {
        auto before_pc = bytecode_table_->Size();
        bytecode_table_->EmitOpcode(OpcodeType::kGoto);
        bytecode_table_->EmitI16(0); // 占位符,稍后修复(使用int16,与RepairPc一致)
        return before_pc;
    }

    /**
     * @brief 辅助方法:验证PC值是否正确
     * @param pc 要验证的PC位置
     * @param expected_pc 期望的PC值
     */
    void VerifyPcValue(Pc pc, Pc expected_pc) {
        // RepairPc存储的是相对偏移,不是绝对PC值
        // 偏移 = 目标PC - 源PC
        auto actual_offset = bytecode_table_->GetI16(pc + 1); // +1跳过操作码
        auto expected_offset = static_cast<int16_t>(expected_pc - pc);
        EXPECT_EQ(actual_offset, expected_offset);
    }

protected:
    std::unique_ptr<test::TestEnvironment> env_;  ///< 测试环境
    FunctionDef* function_def_;                   ///< 函数定义
    BytecodeTable* bytecode_table_;               ///< 字节码表
};

/**
 * @brief 测试RepairEntries处理break语句
 */
TEST_F(JumpManagerTest, RepairBreakEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc break_pc = EmitJumpInstruction();
    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc});

    // Act
    // 发射一些指令,模拟循环体
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc end_pc = bytecode_table_->Size();

    // 修复break跳转
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, kInvalidPc);

    // Assert
    VerifyPcValue(break_pc, end_pc);
}

/**
 * @brief 测试RepairEntries处理continue语句
 */
TEST_F(JumpManagerTest, RepairContinueEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc continue_pc = EmitJumpInstruction();
    entries.push_back({compiler::RepairEntry::Type::kContinue, continue_pc});

    // Act
    Pc loop_start_pc = bytecode_table_->Size();
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc end_pc = bytecode_table_->Size();

    // 修复continue跳转
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, loop_start_pc);

    // Assert
    VerifyPcValue(continue_pc, loop_start_pc);
}

/**
 * @brief 测试RepairEntries处理多个break语句
 */
TEST_F(JumpManagerTest, RepairMultipleBreakEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc break_pc1 = EmitJumpInstruction();
    Pc break_pc2 = EmitJumpInstruction();
    Pc break_pc3 = EmitJumpInstruction();

    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc1});
    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc2});
    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc3});

    // Act
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc end_pc = bytecode_table_->Size();

    // 修复所有break跳转
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, kInvalidPc);

    // Assert
    VerifyPcValue(break_pc1, end_pc);
    VerifyPcValue(break_pc2, end_pc);
    VerifyPcValue(break_pc3, end_pc);
}

/**
 * @brief 测试RepairEntries处理多个continue语句
 */
TEST_F(JumpManagerTest, RepairMultipleContinueEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc continue_pc1 = EmitJumpInstruction();
    Pc continue_pc2 = EmitJumpInstruction();

    entries.push_back({compiler::RepairEntry::Type::kContinue, continue_pc1});
    entries.push_back({compiler::RepairEntry::Type::kContinue, continue_pc2});

    // Act
    Pc loop_start_pc = bytecode_table_->Size();
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc end_pc = bytecode_table_->Size();

    // 修复所有continue跳转
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, loop_start_pc);

    // Assert
    VerifyPcValue(continue_pc1, loop_start_pc);
    VerifyPcValue(continue_pc2, loop_start_pc);
}

/**
 * @brief 测试RepairEntries处理混合的break和continue语句
 */
TEST_F(JumpManagerTest, RepairMixedBreakAndContinueEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc break_pc1 = EmitJumpInstruction();
    Pc continue_pc1 = EmitJumpInstruction();
    Pc break_pc2 = EmitJumpInstruction();
    Pc continue_pc2 = EmitJumpInstruction();

    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc1});
    entries.push_back({compiler::RepairEntry::Type::kContinue, continue_pc1});
    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc2});
    entries.push_back({compiler::RepairEntry::Type::kContinue, continue_pc2});

    // Act
    Pc loop_start_pc = bytecode_table_->Size();
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc end_pc = bytecode_table_->Size();

    // 修复所有跳转
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, loop_start_pc);

    // Assert
    VerifyPcValue(break_pc1, end_pc);
    VerifyPcValue(continue_pc1, loop_start_pc);
    VerifyPcValue(break_pc2, end_pc);
    VerifyPcValue(continue_pc2, loop_start_pc);
}

/**
 * @brief 测试RepairEntries处理空的entries列表
 */
TEST_F(JumpManagerTest, RepairEmptyEntries) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;

    // Act
    Pc end_pc = bytecode_table_->Size();
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, kInvalidPc);

    // Assert - 不应该抛出异常
    EXPECT_EQ(bytecode_table_->Size(), 0);
}

/**
 * @brief 测试RepairEntries处理无效的repair type
 */
TEST_F(JumpManagerTest, RepairInvalidTypeThrowsError) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc jump_pc = EmitJumpInstruction();

    // 创建一个无效的entry类型
    compiler::RepairEntry invalid_entry;
    invalid_entry.type = static_cast<compiler::RepairEntry::Type>(99); // 无效类型
    invalid_entry.repair_pc = jump_pc;
    entries.push_back(invalid_entry);

    // Act & Assert
    compiler::JumpManager manager;
    EXPECT_THROW(
        manager.RepairEntries(function_def_, entries, bytecode_table_->Size(), kInvalidPc),
        SyntaxError
    );
}

/**
 * @brief 测试SetCurrentLoopRepairEntries
 */
TEST_F(JumpManagerTest, SetCurrentLoopRepairEntries) {
    // Arrange
    compiler::JumpManager manager;
    std::vector<compiler::RepairEntry> entries;
    entries.push_back({compiler::RepairEntry::Type::kBreak, 100});

    // Act
    manager.set_current_loop_repair_entries(&entries);

    // Assert
    EXPECT_EQ(manager.current_loop_repair_entries(), &entries);
    EXPECT_EQ(manager.current_loop_repair_entries()->size(), 1);
}

/**
 * @brief 测试GetCurrentLoopRepairEntries默认为nullptr
 */
TEST_F(JumpManagerTest, CurrentLoopRepairEntriesDefaultsToNull) {
    // Arrange
    compiler::JumpManager manager;

    // Act & Assert
    EXPECT_EQ(manager.current_loop_repair_entries(), nullptr);
}

/**
 * @brief 测试LabelMap的基本操作
 */
TEST_F(JumpManagerTest, LabelMapOperations) {
    // Arrange
    compiler::JumpManager manager;
    auto& label_map = manager.label_map();

    // Act
    compiler::LabelInfo info;
    info.current_loop_start_pc = 100;
    info.entries.push_back({compiler::RepairEntry::Type::kBreak, 200});

    label_map["loop1"] = info;

    // Assert
    EXPECT_EQ(label_map.size(), 1);
    EXPECT_EQ(label_map["loop1"].current_loop_start_pc, 100);
    EXPECT_EQ(label_map["loop1"].entries.size(), 1);
}

/**
 * @brief 测试LabelMap访问不存在的标签
 */
TEST_F(JumpManagerTest, LabelMapAccessNonExistentLabel) {
    // Arrange
    compiler::JumpManager manager;
    auto& label_map = manager.label_map();

    // Act & Assert
    EXPECT_EQ(label_map.find("nonexistent"), label_map.end());
}

/**
 * @brief 测试LabelMap更新已存在的标签
 */
TEST_F(JumpManagerTest, LabelMapUpdateExistingLabel) {
    // Arrange
    compiler::JumpManager manager;
    auto& label_map = manager.label_map();

    compiler::LabelInfo info1;
    info1.current_loop_start_pc = 100;
    label_map["loop1"] = info1;

    // Act
    compiler::LabelInfo info2;
    info2.current_loop_start_pc = 200;
    info2.entries.push_back({compiler::RepairEntry::Type::kBreak, 300});
    label_map["loop1"] = info2;

    // Assert
    EXPECT_EQ(label_map["loop1"].current_loop_start_pc, 200);
    EXPECT_EQ(label_map["loop1"].entries.size(), 1);
}

/**
 * @brief 测试CurrentLabelReloopPc的设置和获取
 */
TEST_F(JumpManagerTest, CurrentLabelReloopPcGetterSetter) {
    // Arrange
    compiler::JumpManager manager;

    // Act
    EXPECT_FALSE(manager.current_label_reloop_pc().has_value());
    manager.set_current_label_reloop_pc(100);

    // Assert
    EXPECT_TRUE(manager.current_label_reloop_pc().has_value());
    EXPECT_EQ(manager.current_label_reloop_pc().value(), 100);
}

/**
 * @brief 测试CurrentLabelReloopPc重置
 */
TEST_F(JumpManagerTest, CurrentLabelReloopPcReset) {
    // Arrange
    compiler::JumpManager manager;
    manager.set_current_label_reloop_pc(100);

    // Act
    manager.set_current_label_reloop_pc(std::nullopt);

    // Assert
    EXPECT_FALSE(manager.current_label_reloop_pc().has_value());
}

/**
 * @brief 测试嵌套循环的跳转修复场景
 */
TEST_F(JumpManagerTest, NestedLoopJumpRepair) {
    // Arrange
    compiler::JumpManager manager;
    std::vector<compiler::RepairEntry> outer_entries;
    std::vector<compiler::RepairEntry> inner_entries;

    // 内层循环的跳转
    Pc inner_break = EmitJumpInstruction();
    Pc inner_continue = EmitJumpInstruction();
    inner_entries.push_back({compiler::RepairEntry::Type::kBreak, inner_break});
    inner_entries.push_back({compiler::RepairEntry::Type::kContinue, inner_continue});

    // Act - 修复内层循环
    Pc inner_loop_start = bytecode_table_->Size();
    bytecode_table_->EmitOpcode(OpcodeType::kUndefined);
    Pc inner_end = bytecode_table_->Size();

    manager.RepairEntries(function_def_, inner_entries, inner_end, inner_loop_start);

    // Assert - 内层跳转应该指向内层位置
    VerifyPcValue(inner_break, inner_end);
    VerifyPcValue(inner_continue, inner_loop_start);
}

/**
 * @brief 测试compiler::LabelInfo的entries向量操作
 */
TEST_F(JumpManagerTest, LabelInfoEntriesVector) {
    // Arrange
    compiler::JumpManager manager;
    auto& label_map = manager.label_map();

    compiler::LabelInfo info;
    info.current_loop_start_pc = 50;

    // Act
    for (int i = 0; i < 5; i++) {
        info.entries.push_back({compiler::RepairEntry::Type::kBreak, static_cast<Pc>(100 + i * 10)});
    }

    label_map["test_label"] = info;

    // Assert
    EXPECT_EQ(label_map["test_label"].entries.size(), 5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(label_map["test_label"].entries[i].repair_pc, 100 + i * 10);
    }
}

/**
 * @brief 测试多个LabelInfo的管理
 */
TEST_F(JumpManagerTest, MultipleLabelInfoManagement) {
    // Arrange
    compiler::JumpManager manager;
    auto& label_map = manager.label_map();

    // Act - 添加多个标签
    for (int i = 0; i < 3; i++) {
        compiler::LabelInfo info;
        info.current_loop_start_pc = i * 100;
        info.entries.push_back({compiler::RepairEntry::Type::kBreak, static_cast<Pc>(1000 + i)});
        label_map["label" + std::to_string(i)] = info;
    }

    // Assert
    EXPECT_EQ(label_map.size(), 3);
    EXPECT_EQ(label_map["label0"].current_loop_start_pc, 0);
    EXPECT_EQ(label_map["label1"].current_loop_start_pc, 100);
    EXPECT_EQ(label_map["label2"].current_loop_start_pc, 200);
}

/**
 * @brief 测试Continue跳转到invalidPc的行为
 */
TEST_F(JumpManagerTest, ContinueToInvalidPcWithAssertion) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    entries.push_back({compiler::RepairEntry::Type::kContinue, 0});

    // Act & Assert
    compiler::JumpManager manager;
    // 注意: 传入kInvalidPc作为reloop_pc应该触发assert
    // 在Release模式下可能不会触发,这里只测试Debug模式
    #ifndef NDEBUG
    EXPECT_DEATH(
        manager.RepairEntries(function_def_, entries, 100, kInvalidPc),
        ""
    );
    #endif
}

/**
 * @brief 测试RepairEntries不修改其他PC值
 */
TEST_F(JumpManagerTest, RepairEntriesDoesNotModifyOtherPcs) {
    // Arrange
    std::vector<compiler::RepairEntry> entries;
    Pc break_pc = EmitJumpInstruction();
    entries.push_back({compiler::RepairEntry::Type::kBreak, break_pc});

    // 发射另一条跳转指令,不参与修复
    Pc other_jump_pc = EmitJumpInstruction();
    Pc other_target = 9999;
    bytecode_table_->RepairPc(other_jump_pc, other_target);

    // Act
    Pc end_pc = bytecode_table_->Size();
    compiler::JumpManager manager;
    manager.RepairEntries(function_def_, entries, end_pc, kInvalidPc);

    // Assert
    VerifyPcValue(break_pc, end_pc);
    VerifyPcValue(other_jump_pc, other_target); // 应该保持不变
}

} // namespace mjs

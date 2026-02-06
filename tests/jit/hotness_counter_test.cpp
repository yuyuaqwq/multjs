/**
 * @file hotness_counter_test.cpp
 * @brief HotnessCounter单元测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <mjs/jit/hotness_counter.h>

using namespace mjs::jit;

/**
 * @test 测试热点计数器初始状态
 */
TEST(HotnessCounter, InitialState) {
    HotnessCounter counter;

    EXPECT_EQ(counter.state(), ExecutionState::kInterpreted);
    EXPECT_EQ(counter.count(), 0);
}

/**
 * @test 测试Baseline阈值触发
 */
TEST(HotnessCounter, TriggerBaseline) {
    HotnessCounter counter;

    // 初始状态是解释执行
    EXPECT_EQ(counter.state(), ExecutionState::kInterpreted);

    // 执行99次，不应该触发
    for (int i = 0; i < 99; i++) {
        counter.Increment();
    }
    EXPECT_EQ(counter.state(), ExecutionState::kInterpreted);
    EXPECT_EQ(counter.count(), 99);

    // 第100次触发Baseline编译
    counter.Increment();
    EXPECT_EQ(counter.state(), ExecutionState::kWarmup);
    EXPECT_EQ(counter.count(), 100);
}

/**
 * @test 测试重置计数器
 */
TEST(HotnessCounter, Reset) {
    HotnessCounter counter;

    // 增加计数
    for (int i = 0; i < 50; i++) {
        counter.Increment();
    }
    EXPECT_EQ(counter.count(), 50);

    // 重置
    counter.Reset();
    EXPECT_EQ(counter.count(), 0);
    EXPECT_EQ(counter.state(), ExecutionState::kInterpreted);
}

/**
 * @test 测试状态设置
 */
TEST(HotnessCounter, SetState) {
    HotnessCounter counter;

    EXPECT_EQ(counter.state(), ExecutionState::kInterpreted);

    counter.set_state(ExecutionState::kBaseline);
    EXPECT_EQ(counter.state(), ExecutionState::kBaseline);

    counter.set_state(ExecutionState::kWarmup);
    EXPECT_EQ(counter.state(), ExecutionState::kWarmup);
}

/**
 * @test 测试在Baseline状态下计数器仍然工作
 */
TEST(HotnessCounter, CountInBaselineState) {
    HotnessCounter counter;

    // 触发Baseline
    for (int i = 0; i < 100; i++) {
        counter.Increment();
    }
    EXPECT_EQ(counter.state(), ExecutionState::kWarmup);

    // 设置为Baseline状态
    counter.set_state(ExecutionState::kBaseline);

    // 继续计数
    uint32_t baseline_count = counter.count();
    for (int i = 0; i < 10; i++) {
        counter.Increment();
    }
    EXPECT_EQ(counter.count(), baseline_count + 10);
}

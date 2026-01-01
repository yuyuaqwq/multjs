/**
 * @file stack_frame_test.cpp
 * @brief 栈帧和栈单元测试
 *
 * 测试StackFrame和Stack的功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include <mjs/stack_frame.h>
#include <mjs/value.h>
#include <mjs/runtime.h>
#include <mjs/function_def.h>
#include <mjs/module_def.h>
#include "test_helpers.h"

namespace mjs {
namespace test {

/**
 * @class StackTest
 * @brief 栈测试
 */
class StackTest : public ::testing::Test {
protected:
    void SetUp() override {
        stack_ = std::make_unique<Stack>(1024); // 创建1K大小的栈
    }

    void TearDown() override {
        stack_.reset();
    }

    std::unique_ptr<Stack> stack_;
};

/**
 * @test 测试栈创建
 */
TEST_F(StackTest, StackCreation) {
    // Assert
    ASSERT_NE(stack_, nullptr);
    EXPECT_EQ(stack_->size(), 0);
}

/**
 * @test 测试栈push操作
 */
TEST_F(StackTest, StackPush) {
    // Arrange
    Value value1(42);
    Value value2(3.14);

    // Act
    stack_->push(value1);
    stack_->push(value2);

    // Assert
    EXPECT_EQ(stack_->size(), 2);
}

/**
 * @test 测试栈pop操作
 */
TEST_F(StackTest, StackPop) {
    // Arrange
    Value value1(42);
    Value value2(100);
    stack_->push(value1);
    stack_->push(value2);

    // Act
    Value popped = stack_->pop();

    // Assert
    EXPECT_EQ(popped.ToInt64(), 100);
    EXPECT_EQ(stack_->size(), 1);
}

/**
 * @test 测试栈get操作
 */
TEST_F(StackTest, StackGet) {
    // Arrange
    Value value1(1);
    Value value2(2);
    Value value3(3);
    stack_->push(value1);
    stack_->push(value2);
    stack_->push(value3);

    // Act
    Value& value = stack_->get(1);

    // Assert
    EXPECT_EQ(value.ToInt64(), 2);
}

/**
 * @test 测试栈set操作
 */
TEST_F(StackTest, StackSet) {
    // Arrange
    Value value1(1);
    Value value2(2);
    stack_->push(value1);
    stack_->push(value2);

    // Act
    Value new_value(99);
    stack_->set(0, new_value);

    // Assert
    Value& retrieved = stack_->get(0);
    EXPECT_EQ(retrieved.ToInt64(), 99);
}

/**
 * @test 测试栈upgrade操作
 */
TEST_F(StackTest, StackUpgrade) {
    // Arrange
    stack_->push(Value(1));
    stack_->push(Value(2));
    EXPECT_EQ(stack_->size(), 2);

    // Act
    stack_->upgrade(3);

    // Assert
    EXPECT_EQ(stack_->size(), 5); // 2 + 3
}

/**
 * @test 测试栈reduce操作
 */
TEST_F(StackTest, StackReduce) {
    // Arrange
    stack_->push(Value(1));
    stack_->push(Value(2));
    stack_->push(Value(3));
    stack_->push(Value(4));
    EXPECT_EQ(stack_->size(), 4);

    // Act
    stack_->reduce(2);

    // Assert
    EXPECT_EQ(stack_->size(), 2); // 4 - 2
}

/**
 * @test 测试栈resize操作
 */
TEST_F(StackTest, StackResize) {
    // Arrange
    stack_->push(Value(1));
    stack_->push(Value(2));
    stack_->push(Value(3));

    // Act
    stack_->resize(5);

    // Assert
    EXPECT_EQ(stack_->size(), 5);
}

/**
 * @test 测试栈clear操作
 */
TEST_F(StackTest, StackClear) {
    // Arrange
    stack_->push(Value(1));
    stack_->push(Value(2));
    stack_->push(Value(3));
    EXPECT_EQ(stack_->size(), 3);

    // Act
    stack_->clear();

    // Assert
    EXPECT_EQ(stack_->size(), 0);
}

/**
 * @test 测试栈vector访问
 */
TEST_F(StackTest, StackVectorAccess) {
    // Arrange
    stack_->push(Value(1));
    stack_->push(Value(2));

    // Act
    auto& vector = stack_->vector();

    // Assert
    EXPECT_EQ(vector.size(), 2);
}

/**
 * @class StackFrameTest
 * @brief 栈帧测试
 */
class StackFrameTest : public ::testing::Test {
protected:
    void SetUp() override {
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
    }

    void TearDown() override {
        stack_frame_.reset();
        stack_.reset();
    }

    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
};

/**
 * @test 测试栈帧创建
 */
TEST_F(StackFrameTest, StackFrameCreation) {
    // Assert
    ASSERT_NE(stack_frame_, nullptr);
    EXPECT_EQ(stack_frame_->bottom(), 0);
}

/**
 * @test 测试栈帧push操作
 */
TEST_F(StackFrameTest, StackFramePush) {
    // Arrange
    Value value1(42);
    Value value2(100);

    // Act
    stack_frame_->push(value1);
    stack_frame_->push(value2);

    // Assert - 栈的大小应该增加
    EXPECT_GE(stack_->size(), 2);
}

/**
 * @test 测试栈帧pop操作
 */
TEST_F(StackFrameTest, StackFramePop) {
    // Arrange
    stack_frame_->push(Value(42));
    stack_frame_->push(Value(100));

    // Act
    Value popped = stack_frame_->pop();

    // Assert
    EXPECT_EQ(popped.ToInt64(), 100);
}

/**
 * @test 测试栈帧get操作(正索引)
 */
TEST_F(StackFrameTest, StackFrameGetPositiveIndex) {
    // Arrange
    stack_frame_->push(Value(10));
    stack_frame_->push(Value(20));
    stack_frame_->push(Value(30));

    // Act
    Value& value = stack_frame_->get(0); // 从栈帧底向上

    // Assert
    EXPECT_EQ(value.ToInt64(), 10);
}

/**
 * @test 测试栈帧get操作(负索引)
 */
TEST_F(StackFrameTest, StackFrameGetNegativeIndex) {
    // Arrange
    stack_frame_->push(Value(10));
    stack_frame_->push(Value(20));
    stack_frame_->push(Value(30));

    // Act
    Value& value = stack_frame_->get(-1); // 从栈帧顶向下

    // Assert
    EXPECT_EQ(value.ToInt64(), 30);
}

/**
 * @test 测试栈帧set操作
 */
TEST_F(StackFrameTest, StackFrameSet) {
    // Arrange
    stack_frame_->push(Value(10));
    stack_frame_->push(Value(20));

    // Act
    stack_frame_->set(0, Value(99));

    // Assert
    Value& value = stack_frame_->get(0);
    EXPECT_EQ(value.ToInt64(), 99);
}

/**
 * @test 测试栈帧upgrade操作
 */
TEST_F(StackFrameTest, StackFrameUpgrade) {
    // Arrange
    stack_frame_->push(Value(1));
    stack_frame_->push(Value(2));
    auto size_before = stack_->size();

    // Act
    stack_frame_->upgrade(3);

    // Assert
    EXPECT_EQ(stack_->size(), size_before + 3);
}

/**
 * @test 测试栈帧reduce操作
 */
TEST_F(StackFrameTest, StackFrameReduce) {
    // Arrange
    stack_frame_->push(Value(1));
    stack_frame_->push(Value(2));
    stack_frame_->push(Value(3));
    stack_frame_->push(Value(4));
    auto size_before = stack_->size();

    // Act
    stack_frame_->reduce(2);

    // Assert
    EXPECT_EQ(stack_->size(), size_before - 2);
}

/**
 * @test 测试栈帧bottom设置
 */
TEST_F(StackFrameTest, StackFrameBottom) {
    // Arrange
    stack_frame_->push(Value(1));
    stack_frame_->push(Value(2));

    // Act
    stack_frame_->set_bottom(2);

    // Assert
    EXPECT_EQ(stack_frame_->bottom(), 2);
}

/**
 * @test 测试栈帧upper_stack_frame
 */
TEST_F(StackFrameTest, StackFrameUpperStackFrame) {
    // Arrange - 创建第一个栈帧
    StackFrame frame1(stack_.get());

    // Act - 创建第二个栈帧,指向第一个
    StackFrame frame2(&frame1);

    // Assert
    EXPECT_EQ(frame2.upper_stack_frame(), &frame1);
}

/**
 * @class StackFrameFunctionTest
 * @brief 栈帧函数相关测试
 */
class StackFrameFunctionTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        stack_ = std::make_unique<Stack>(1024);
        stack_frame_ = std::make_unique<StackFrame>(stack_.get());
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
        function_def_ = TestFunctionDef::CreateShared(module_def_.get(), "test_function", 2);
    }

    void TearDown() override {
        function_def_.reset();
        module_def_.reset();
        stack_frame_.reset();
        stack_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Stack> stack_;
    std::unique_ptr<StackFrame> stack_frame_;
    std::shared_ptr<ModuleDef> module_def_;
    std::shared_ptr<FunctionDef> function_def_;
};

/**
 * @test 测试设置函数值
 */
TEST_F(StackFrameFunctionTest, SetFunctionVal) {
    // Arrange
    Value func_val(ValueType::kFunctionObject);

    // Act
    stack_frame_->set_function_val(std::move(func_val));

    // Assert
    EXPECT_EQ(stack_frame_->function_val().type(), ValueType::kFunctionObject);
}

/**
 * @test 测试设置函数定义
 */
TEST_F(StackFrameFunctionTest, SetFunctionDef) {
    // Act
    stack_frame_->set_function_def(function_def_.get());

    // Assert
    EXPECT_EQ(stack_frame_->function_def(), function_def_.get());
}

/**
 * @test 测试设置this值
 */
TEST_F(StackFrameFunctionTest, SetThisVal) {
    // Arrange
    Value this_val(ValueType::kObject);

    // Act
    stack_frame_->set_this_val(std::move(this_val));

    // Assert
    EXPECT_EQ(stack_frame_->this_val().type(), ValueType::kObject);
}

/**
 * @test 测试设置和获取pc
 */
TEST_F(StackFrameFunctionTest, SetAndGetPC) {
    // Arrange
    Pc pc = 100;

    // Act
    stack_frame_->set_pc(pc);

    // Assert
    EXPECT_EQ(stack_frame_->pc(), 100);
}

/**
 * @class StackFrameIntegrationTest
 * @brief 栈帧集成测试
 */
class StackFrameIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        runtime_ = TestRuntime::Create();
        stack_ = std::make_unique<Stack>(1024);
        module_def_ = TestModuleDef::CreateShared(runtime_.get(), "test_module");
    }

    void TearDown() override {
        module_def_.reset();
        stack_.reset();
        runtime_.reset();
    }

    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Stack> stack_;
    std::shared_ptr<ModuleDef> module_def_;
};

/**
 * @test 测试多层栈帧嵌套
 */
TEST_F(StackFrameIntegrationTest, NestedStackFrames) {
    // Arrange - 创建多层栈帧
    StackFrame frame1(stack_.get());
    frame1.push(Value(1));
    frame1.push(Value(2));

    StackFrame frame2(&frame1);
    frame2.push(Value(3));
    frame2.push(Value(4));

    StackFrame frame3(&frame2);
    frame3.push(Value(5));
    frame3.push(Value(6));

    // Assert - 验证栈帧的嵌套关系
    EXPECT_EQ(frame2.upper_stack_frame(), &frame1);
    EXPECT_EQ(frame3.upper_stack_frame(), &frame2);
}

/**
 * @test 测试函数调用栈模拟
 */
TEST_F(StackFrameIntegrationTest, FunctionCallSimulation) {
    // Arrange - 主函数栈帧
    StackFrame main_frame(stack_.get());
    auto* main_func = TestFunctionDef::Create(module_def_.get(), "main", 0);
    main_frame.set_function_def(main_func);
    main_frame.push(Value(10));
    main_frame.push(Value(20));

    // Act - 调用子函数
    StackFrame sub_frame(&main_frame);
    auto* sub_func = TestFunctionDef::Create(module_def_.get(), "sub", 2);
    sub_frame.set_function_def(sub_func);

    // Assert
    EXPECT_EQ(sub_frame.upper_stack_frame(), &main_frame);
    EXPECT_EQ(main_frame.function_def(), main_func);
    EXPECT_EQ(sub_frame.function_def(), sub_func);
}

/**
 * @test 测试栈帧在函数调用中的状态保持
 */
TEST_F(StackFrameIntegrationTest, StackFrameStatePreservation) {
    // Arrange
    StackFrame frame1(stack_.get());
    frame1.push(Value(100));
    frame1.set_bottom(0);

    // Act - 创建子栈帧并操作
    StackFrame frame2(&frame1);
    frame2.push(Value(200));
    frame2.push(Value(300));

    // Assert - 验证frame1的状态没有被破坏
    EXPECT_EQ(frame1.bottom(), 0);
    EXPECT_GE(stack_->size(), 3); // 至少有3个值
}

/**
 * @test 测试栈帧的非拷贝性
 */
TEST_F(StackFrameIntegrationTest, StackFrameNonCopyable) {
    // StackFrame继承自noncopyable
    EXPECT_TRUE(std::is_copy_assignable<StackFrame>::value == false);
    EXPECT_TRUE(std::is_copy_constructible<StackFrame>::value == false);
}

/**
 * @test 测试Stack的非拷贝性
 */
TEST_F(StackFrameIntegrationTest, StackNonCopyable) {
    // Stack继承自noncopyable
    EXPECT_TRUE(std::is_copy_assignable<Stack>::value == false);
    EXPECT_TRUE(std::is_copy_constructible<Stack>::value == false);
}

} // namespace test
} // namespace mjs

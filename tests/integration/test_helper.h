/**
 * @file test_helper.h
 * @brief 集成测试辅助工具类
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <fstream>

#include <mjs/runtime.h>

namespace mjs::test {

/**
 * @class IntegrationTestHelper
 * @brief 集成测试辅助类，提供测试环境和常用测试方法
 *
 * 提供 Runtime 和 Context 的创建、JavaScript代码执行、结果验证等功能
 */
class IntegrationTestHelper : public ::testing::Test {
protected:
    /**
     * @brief 测试设置，在每个测试用例开始前调用
     */
    void SetUp() override;

    /**
     * @brief 测试清理，在每个测试用例结束后调用
     */
    void TearDown() override;

    /**
     * @brief 创建Runtime实例
     * @return Runtime指针
     */
    Runtime* runtime() { return runtime_.get(); }

    /**
     * @brief 创建Context实例
     * @return Context指针
     */
    Context* context() { return context_.get(); }

    /**
     * @brief 执行JavaScript代码
     * @param code JavaScript源代码
     * @return 执行结果Value
     */
    Value Exec(const std::string& code);

    /**
     * @brief 执行JavaScript代码（指定模块名）
     * @param module_name 模块名称
     * @param code JavaScript源代码
     * @return 执行结果Value
     */
    Value Exec(const std::string& module_name, const std::string& code);

    /**
     * @brief 从文件执行JavaScript代码
     * @param file_path 文件路径（相对于tests/integration/fixtures/）
     * @return 执行结果Value
     */
    Value ExecFromFile(const std::string& file_path);

    /**
     * @brief 断言执行结果等于期望值
     * @param code JavaScript代码
     * @param expected 期望的结果值
     */
    void AssertEq(const std::string& code, Value expected);

    /**
     * @brief 断言执行结果等于期望值（指定模块名）
     * @param module_name 模块名称
     * @param code JavaScript代码
     * @param expected 期望的结果值
     */
    void AssertEq(const std::string& module_name, const std::string& code, Value expected);

    /**
     * @brief 断言执行结果为布尔值true
     * @param code JavaScript代码
     */
    void AssertTrue(const std::string& code);

    /**
     * @brief 断言执行结果为布尔值false
     * @param code JavaScript代码
     */
    void AssertFalse(const std::string& code);

    /**
     * @brief 断言执行结果为null
     * @param code JavaScript代码
     */
    void AssertNull(const std::string& code);

    /**
     * @brief 断言执行结果为undefined
     * @param code JavaScript代码
     */
    void AssertUndefined(const std::string& code);

    /**
     * @brief 断言执行结果抛出异常
     * @param code JavaScript代码
     */
    void AssertThrows(const std::string& code);

    /**
     * @brief 断言执行结果抛出异常（指定模块名）
     * @param module_name 模块名称
     * @param code JavaScript代码
     */
    void AssertThrows(const std::string& module_name, const std::string& code);

    /**
     * @brief 执行微任务队列
     * 用于处理Promise等异步操作
     */
    void RunMicrotasks();

    /**
     * @brief 重置测试环境
     * 创建新的Runtime和Context实例
     */
    void Reset();

    /**
     * @brief 生成唯一的模块名
     * @return 唯一的模块名
     */
    std::string GenerateModuleName();

    /**
     * @brief 获取fixtures目录路径
     * @return fixtures目录的绝对路径
     */
    std::string GetFixturePath(const std::string& relative_path = "");

    /**
     * @brief 读取fixture文件内容
     * @param file_path 相对于fixtures目录的文件路径
     * @return 文件内容
     */
    std::string ReadFixture(const std::string& file_path);

private:
    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    uint32_t module_counter_ = 0;
};

} // namespace mjs::test

/**
 * @file test_helper.cpp
 * @brief 集成测试辅助工具类实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <filesystem>
#include <sstream>

namespace mjs::test {

void IntegrationTestHelper::SetUp() {
    // 创建Runtime实例
    runtime_ = std::make_unique<Runtime>();

    // 创建Context实例
    context_ = std::make_unique<Context>(runtime_.get());

    module_counter_ = 0;
}

void IntegrationTestHelper::TearDown() {
    // 清理顺序：先Context后Runtime
    context_.reset();
    runtime_.reset();
}

Value IntegrationTestHelper::Exec(const std::string& code) {
    return Exec(GenerateModuleName(), code);
}

Value IntegrationTestHelper::Exec(const std::string& module_name, const std::string& code) {
    try {
        return context_->Eval(module_name, code);
    } catch (const std::exception& e) {
        throw std::runtime_error("执行代码失败: " + std::string(e.what()) + "\n代码: " + code);
    }
}

Value IntegrationTestHelper::ExecFromFile(const std::string& file_path) {
    std::string full_path = GetFixturePath(file_path);
    std::string code = ReadFixture(file_path);
    return Exec(file_path, code);
}

void IntegrationTestHelper::AssertEq(const std::string& code, Value expected) {
    AssertEq(GenerateModuleName(), code, expected);
}

void IntegrationTestHelper::AssertEq(const std::string& module_name, const std::string& code, Value expected) {
    Value result = Exec(module_name, code);

    if (result.IsException()) {
        FAIL() << "exception: " << result.string_view();
    }

    // 根据类型进行比较
    if (expected.IsNumber() && result.IsNumber()) {
        EXPECT_DOUBLE_EQ(expected.ToNumber().f64(), result.ToNumber().f64())
            << "代码: " << code;
    } else if (expected.IsBoolean() && result.IsBoolean()) {
        EXPECT_EQ(expected.boolean(), result.boolean())
            << "代码: " << code;
    } else if (expected.IsString() && result.IsString()) {
        EXPECT_EQ(std::string(expected.string_view()), result.string_view())
            << "代码: " << code;
    } else if (expected.IsNull() && result.IsNull()) {
        SUCCEED();
    } else if (expected.IsUndefined() && result.IsUndefined()) {
        SUCCEED();
    } else if (expected.IsObject() && result.IsObject()) {
        // 对于对象类型，简单比较是否为同一个对象
        EXPECT_EQ(&expected.object(), &result.object())
            << "代码: " << code;
    } else {
        FAIL() << "类型不匹配。期望: " << expected.TypeToString(expected.type())
               << ", 实际: " << result.TypeToString(result.type());
    }
}

void IntegrationTestHelper::AssertTrue(const std::string& code) {
    AssertEq(code, Value(true));
}

void IntegrationTestHelper::AssertFalse(const std::string& code) {
    AssertEq(code, Value(false));
}

void IntegrationTestHelper::AssertNull(const std::string& code) {
    AssertEq(code, Value(nullptr));
}

void IntegrationTestHelper::AssertUndefined(const std::string& code) {
    AssertEq(code, Value());
}

void IntegrationTestHelper::AssertThrows(const std::string& code) {
    AssertThrows(GenerateModuleName(), code);
}

void IntegrationTestHelper::AssertThrows(const std::string& module_name, const std::string& code) {
    EXPECT_THROW({
        Exec(module_name, code);
    }, std::exception) << "代码应该抛出异常: " << code;
}

void IntegrationTestHelper::RunMicrotasks() {
    context_->ExecuteMicrotasks();
}

void IntegrationTestHelper::Reset() {
    context_.reset();
    runtime_.reset();

    runtime_ = std::make_unique<Runtime>();
    context_ = std::make_unique<Context>(runtime_.get());
    module_counter_ = 0;
}

std::string IntegrationTestHelper::GenerateModuleName() {
    std::ostringstream oss;
    oss << "test_module_" << module_counter_++;
    return oss.str();
}

std::string IntegrationTestHelper::GetFixturePath(const std::string& relative_path) {
    // 获取当前文件所在目录
    std::filesystem::path current_file = __FILE__;
    std::filesystem::path current_dir = current_file.parent_path();

    // 构建fixtures目录路径
    std::filesystem::path fixtures_dir = current_dir / "fixtures";
    if (!relative_path.empty()) {
        fixtures_dir /= relative_path;
    }

    return fixtures_dir.string();
}

std::string IntegrationTestHelper::ReadFixture(const std::string& file_path) {
    std::string full_path = GetFixturePath(file_path);
    std::ifstream file(full_path);

    if (!file.is_open()) {
        throw std::runtime_error("无法打开fixture文件: " + full_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace mjs::test

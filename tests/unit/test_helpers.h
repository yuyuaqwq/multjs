/**
 * @file test_helpers.h
 * @brief 单元测试辅助工具
 *
 * 提供简化的测试辅助类,用于创建测试所需的Runtime、ModuleDef、FunctionDef等对象,
 * 避免在测试中重复设置复杂的依赖关系。
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/module_def.h>
#include <mjs/value/function_def.h>

namespace mjs {
namespace test {

/**
 * @class TestRuntime
 * @brief 测试用Runtime辅助类
 *
 * 提供简化的Runtime初始化,用于测试环境。
 */
class TestRuntime {
public:
    /**
     * @brief 创建测试用Runtime
     * @return Runtime对象
     */
    static std::unique_ptr<Runtime> Create() {
        return std::make_unique<Runtime>();
    }
};

/**
 * @class TestModuleDef
 * @brief 测试用ModuleDef辅助类
 *
 * 提供简化的ModuleDef创建,用于测试环境。
 */
class TestModuleDef {
public:
    /**
     * @brief 创建测试用ModuleDef
     * @param runtime 运行时环境指针
     * @param name 模块名称(默认为"test_module")
     * @return ModuleDef对象指针
     */
    static ModuleDef* Create(Runtime* runtime, const std::string& name = "test_module") {
        return ModuleDef::New(runtime, name, "", 0);
    }

    /**
     * @brief 创建测试用ModuleDef,使用Value管理
     * @param runtime 运行时环境指针
     * @param name 模块名称(默认为"test_module")
     * @return ModuleDef的Value包装
     */
    static Value CreateValue(Runtime* runtime, const std::string& name = "test_module") {
        return Value(Create(runtime, name));
    }
};

/**
 * @class TestFunctionDef
 * @brief 测试用FunctionDef辅助类
 *
 * 提供简化的FunctionDef创建,用于测试环境。
 */
class TestFunctionDef {
public:
    /**
     * @brief 创建测试用FunctionDef
     * @param module_def 所属模块定义指针
     * @param name 函数名称(默认为"test_function")
     * @param param_count 参数数量(默认为0)
     * @return FunctionDef对象指针
     */
    static FunctionDef* Create(ModuleDef* module_def, const std::string& name = "test_function", uint32_t param_count = 0) {
        auto* function_def = FunctionDef::New(module_def, name, param_count);
        // 为参数添加变量定义,确保 var_count >= param_count
        for (uint32_t i = 0; i < param_count; ++i) {
            function_def->var_def_table().AddVar("param_" + std::to_string(i));
        }
        return function_def;
    }

    /**
     * @brief 创建测试用FunctionDef,使用Value管理
     * @param module_def 所属模块定义指针
     * @param name 函数名称(默认为"test_function")
     * @param param_count 参数数量(默认为0)
     * @return FunctionDef的Value包装
     */
    static Value CreateValue(ModuleDef* module_def, const std::string& name = "test_function", uint32_t param_count = 0) {
        return Value(Create(module_def, name, param_count));
    }
};

/**
 * @class TestEnvironment
 * @brief 完整的测试环境
 *
 * 封装Runtime、ModuleDef和FunctionDef的创建,提供一站式测试环境。
 */
class TestEnvironment {
public:
    /**
     * @brief 构造测试环境
     * 自动创建Runtime、ModuleDef和FunctionDef
     */
    TestEnvironment()
        : runtime_(TestRuntime::Create())
        , module_def_(TestModuleDef::CreateValue(runtime_.get(), "test_module"))
        , function_def_(TestFunctionDef::CreateValue(&module_def_.module_def(), "test_function", 0))
    {}

    /**
     * @brief 获取Runtime指针
     * @return Runtime指针
     */
    Runtime* runtime() { return runtime_.get(); }

    /**
     * @brief 获取ModuleDef指针
     * @return ModuleDef指针
     */
    ModuleDef* module_def() { return &module_def_.module_def(); }

    /**
     * @brief 获取FunctionDef指针
     * @return FunctionDef指针
     */
    FunctionDef* function_def() { return &function_def_.function_def(); }

    /**
     * @brief 创建新的FunctionDef
     * @param name 函数名称
     * @param param_count 参数数量
     * @return FunctionDef对象指针
     */
    FunctionDef* CreateFunctionDef(const std::string& name, uint32_t param_count = 0) {
        return TestFunctionDef::Create(module_def(), name, param_count);
    }

private:
    std::unique_ptr<Runtime> runtime_;
    Value module_def_;
    Value function_def_;
};

} // namespace test
} // namespace mjs

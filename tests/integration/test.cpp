#include <Windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/object_impl/cpp_module_object.h>


// 测试文件列表
static const std::vector<std::string> test_files = {
    // 基础功能测试
    "branch.js",
    "closure.js",
    "object.js",
    "exception.js",

    // 异步功能测试
    "async.js",

    // 生成器和迭代器测试
    "generator.js",
    "iterator.ts",

    // 函数测试
    "arrow_function_test.js",

    // GC测试
    "gc.js",

    // 模块测试
    "module1.js",
    "module2.js",

    // Class相关测试
    "class_simple.js",
    "class.js",
    "class.ts",
    "class_execution.js",
    "class_advanced.js",
    "class_edge_cases.js",
};

// 颜色输出辅助函数
namespace Colors {
    const char* RESET = "\033[0m";
    const char* RED = "\033[31m";
    const char* GREEN = "\033[32m";
    const char* YELLOW = "\033[33m";
    const char* BLUE = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN = "\033[36m";
}

struct TestResult {
    std::string file_name;
    bool success;
    std::string error_message;
    double elapsed_ms;
};

// 运行单个测试文件
TestResult RunTest(mjs::Context& ctx, const std::string& filename) {
    TestResult result;
    result.file_name = filename;

    auto start = std::chrono::high_resolution_clock::now();

    try {
        auto module = ctx.EvalFromFile(filename);

        if (module.IsException()) {
            result.success = false;
            result.error_message = std::string(module.string_view());
        } else {
            // 执行微任务队列
            ctx.ExecuteMicrotasks();
            result.success = true;
            result.error_message = "";
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    } catch (...) {
        result.success = false;
        result.error_message = "Unknown exception";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    result.elapsed_ms = elapsed.count();

    return result;
}

// 打印测试结果
void PrintTestResult(const TestResult& result) {
    if (result.success) {
        std::cout << Colors::GREEN << "[PASS]" << Colors::RESET << " ";
        std::cout << result.file_name;
        std::cout << " (" << Colors::CYAN << result.elapsed_ms << "ms" << Colors::RESET << ")";
        std::cout << std::endl;
    } else {
        std::cout << Colors::RED << "[FAIL]" << Colors::RESET << " ";
        std::cout << result.file_name;
        std::cout << " (" << Colors::CYAN << result.elapsed_ms << "ms" << Colors::RESET << ")";
        std::cout << std::endl;
        std::cout << Colors::YELLOW << "  Error: " << result.error_message << Colors::RESET << std::endl;
    }
}

// 打印测试汇总
void PrintSummary(const std::vector<TestResult>& results) {
    int passed = 0;
    int failed = 0;
    double total_time = 0;

    for (const auto& result : results) {
        if (result.success) {
            passed++;
        } else {
            failed++;
        }
        total_time += result.elapsed_ms;
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << Colors::MAGENTA << "Test Summary" << Colors::RESET << "\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Total tests: " << results.size() << "\n";
    std::cout << Colors::GREEN << "Passed: " << passed << Colors::RESET << "\n";
    std::cout << Colors::RED << "Failed: " << failed << Colors::RESET << "\n";
    std::cout << "Total time: " << total_time << "ms\n";
    std::cout << "Average time: " << (total_time / results.size()) << "ms\n";
    std::cout << std::string(60, '=') << "\n";

    if (failed == 0) {
        std::cout << Colors::GREEN << "All tests passed!" << Colors::RESET << "\n";
    } else {
        std::cout << Colors::RED << "Some tests failed!" << Colors::RESET << "\n";
    }
}

int main(int argc, char* argv[]) {
    using namespace mjs;

    // 解析命令行参数
    std::string specific_test;
    bool verbose = false;
    bool run_all = true;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options] [test_file]\n";
            std::cout << "Options:\n";
            std::cout << "  --help, -h     Show this help message\n";
            std::cout << "  --verbose, -v  Enable verbose output\n";
            std::cout << "  --all, -a      Run all tests (default)\n";
            std::cout << "  [test_file]    Run specific test file\n";
            return 0;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--all" || arg == "-a") {
            run_all = true;
        } else {
            specific_test = arg;
            run_all = false;
        }
    }

    // 创建运行时和上下文
    Runtime rt;
    auto ctx = Context(&rt);

    std::vector<TestResult> results;

    std::cout << Colors::CYAN << "MultJS Integration Tests" << Colors::RESET << "\n";
    std::cout << std::string(60, '=') << "\n\n";

    if (run_all) {
        // 运行所有测试
        for (const auto& test_file : test_files) {
            std::cout << Colors::BLUE << "Running: " << Colors::RESET << test_file << "\n";

            auto result = RunTest(ctx, test_file);
            results.push_back(result);

            PrintTestResult(result);

            // 清理模块缓存以便下一个测试
            rt.module_manager().ClearModuleCache();
            rt.stack().resize(0);

            std::cout << std::endl;

            // 如果测试失败且不是verbose模式，询问是否继续
            if (!result.success && !verbose) {
                std::cout << Colors::YELLOW << "Test failed. Continue? (y/n): " << Colors::RESET;
                char choice;
                std::cin >> choice;
                if (choice != 'y' && choice != 'Y') {
                    break;
                }
            }
        }
    } else {
        // 运行特定测试
        if (!specific_test.empty()) {
            std::cout << Colors::BLUE << "Running: " << Colors::RESET << specific_test << "\n";
            auto result = RunTest(ctx, specific_test);
            results.push_back(result);
            PrintTestResult(result);
        } else {
            std::cout << Colors::RED << "Error: No test file specified" << Colors::RESET << "\n";
            return 1;
        }
    }

    // 打印测试汇总
    if (!results.empty()) {
        PrintSummary(results);

        // 如果所有测试都通过，返回成功代码
        return (results.size() == std::count_if(results.begin(), results.end(),
            [](const TestResult& r) { return r.success; })) ? 0 : 1;
    }

    return 0;
}

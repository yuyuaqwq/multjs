/**
 * @file js2cpp.cpp
 * @brief JavaScript到C++转译器命令行工具
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>


#include "cpp_gen/cpp_code_generator.h"
#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"

namespace mjs {
namespace compiler {
namespace cpp_gen {
namespace tools {

/**
 * @brief 打印使用说明
 */
void PrintUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] <input.js>\n"
              << "\n"
              << "Options:\n"
              << "  -o <file>       Output file (default: stdout)\n"
              << "  --namespace     Set namespace name (default: mjs_generated)\n"
              << "  --no-type-inference  Disable type inference\n"
              << "  --indent <n>    Set indent size (default: 4)\n"
              << "  --wrap-global   Wrap global code in initialization function (default: disabled)\n"
              << "  --init-name     Set initialization function name (default: initialize)\n"
              << "  -h, --help      Show this help message\n"
              << "\n"
              << "Examples:\n"
              << "  " << program_name << " input.js -o output.cpp\n"
              << "  " << program_name << " input.js --namespace game_logic\n"
              << "  " << program_name << " input.js --no-type-inference -o output.cpp\n"
              << "  " << program_name << " input.js --wrap-global --init-name setup\n";
}

/**
 * @brief 读取文件内容
 */
std::string ReadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief 写入文件内容
 */
void WriteFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file: " << filename << std::endl;
        exit(1);
    }

    file << content;
}

/**
 * @brief 主函数
 */
int Main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }

    // 解析命令行参数
    std::string input_file;
    std::string output_file;
    std::string namespace_name = "mjs_generated";
    bool enable_type_inference = true;
    int indent_size = 4;
    bool wrap_global_code = true;
    std::string init_function_name = "initialize";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        }

        if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
            continue;
        }

        if (arg == "--namespace" && i + 1 < argc) {
            namespace_name = argv[++i];
            continue;
        }

        if (arg == "--no-type-inference") {
            enable_type_inference = false;
            continue;
        }

        if (arg == "--indent" && i + 1 < argc) {
            indent_size = std::stoi(argv[++i]);
            continue;
        }

        if (arg == "--init-name" && i + 1 < argc) {
            init_function_name = argv[++i];
            continue;
        }

        // 其他参数作为输入文件
        if (input_file.empty()) {
            input_file = arg;
        } else {
            std::cerr << "Error: Multiple input files specified" << std::endl;
            return 1;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        return 1;
    }

    // 读取输入文件
    std::string js_code = ReadFile(input_file);

    try {
        // 词法分析
        mjs::compiler::Lexer lexer(js_code);

        // 语法分析
        mjs::compiler::Parser parser(&lexer);
        parser.ParseProgram();

        // 配置代码生成器
        CppCodeGeneratorConfig config;
        config.namespace_name = namespace_name;
        config.enable_type_inference = enable_type_inference;
        config.indent_size = indent_size;
        config.wrap_global_code = wrap_global_code;
        config.init_function_name = init_function_name;

        // 生成C++代码
        CppCodeGenerator generator(config);
        std::string cpp_code = generator.Generate(parser);

        // 输出结果
        if (output_file.empty()) {
            std::cout << cpp_code;
        } else {
            WriteFile(output_file, cpp_code);
            std::cout << "Successfully generated C++ code: " << output_file << std::endl;
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

} // namespace tools
} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

int main(int argc, char* argv[]) {
    return mjs::compiler::cpp_gen::tools::Main(argc, argv);
}

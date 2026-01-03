/**
 * @file cpp_code_generator.h
 * @brief C++代码生成器，将JavaScript AST转换为C++代码
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "cpp_gen/code_emitter.h"
#include "cpp_gen/type_inference_engine.h"
#include "cpp_gen/name_mangler.h"
#include "cpp_gen/cpp_type.h"

namespace mjs {

namespace compiler {

class Parser;
class Expression;
class Statement;

namespace cpp_gen {

/**
 * @struct CppCodeGeneratorConfig
 * @brief C++代码生成器配置
 */
struct CppCodeGeneratorConfig {
    bool enable_type_inference = true;      ///< 是否启用类型推断
    std::string namespace_name = "mjs_generated";  ///< 生成代码的命名空间
    bool use_std_optional = true;           ///< 是否使用std::optional
    int indent_size = 4;                    ///< 缩进空格数
    bool wrap_global_code = true;          ///< 是否将全局代码包装到函数中
    std::string init_function_name = "initialize";  ///< 全局代码包装的函数名
};

/**
 * @class CppCodeGenerator
 * @brief C++代码生成器主类
 */
class CppCodeGenerator {
public:
    /**
     * @brief 构造函数
     * @param config 配置
     */
    explicit CppCodeGenerator(const CppCodeGeneratorConfig& config = {});

    /**
     * @brief 析构函数
     */
    ~CppCodeGenerator() = default;

    /**
     * @brief 生成C++代码（主入口）
     * @param parser 解析器
     * @return 生成的C++代码字符串
     */
    std::string Generate(const Parser& parser);

    /**
     * @brief 生成表达式代码
     * @param expr 表达式AST节点
     * @param os 输出流
     */
    void GenerateExpression(const Expression* expr, std::ostream& os);

    /**
     * @brief 生成语句代码
     * @param stmt 语句AST节点
     * @param os 输出流
     */
    void GenerateStatement(const Statement* stmt, std::ostream& os);

    /**
     * @brief 获取配置
     */
    const CppCodeGeneratorConfig& config() const { return config_; }

    /**
     * @brief 获取类型推断引擎
     */
    TypeInferenceEngine& type_engine() { return type_engine_; }

    /**
     * @brief 获取名称修饰器
     */
    NameMangler& name_mangler() { return name_mangler_; }

private:
    /**
     * @brief 生成文件头部
     */
    void GenerateHeader();

    /**
     * @brief 生成命名空间开始
     */
    void GenerateNamespaceStart();

    /**
     * @brief 生成命名空间结束
     */
    void GenerateNamespaceEnd();

    /**
     * @brief 生成所有对象结构体定义
     */
    void GenerateStructDefinitions();

    /**
     * @brief 生成全局初始化函数开始
     */
    void GenerateInitFunctionStart();

    /**
     * @brief 生成全局初始化函数结束
     */
    void GenerateInitFunctionEnd();

    /**
     * @brief 生成各种表达式类型
     */
    void GenerateIntegerLiteral(const Expression* expr, std::ostream& os);
    void GenerateFloatLiteral(const Expression* expr, std::ostream& os);
    void GenerateStringLiteral(const Expression* expr, std::ostream& os);
    void GenerateBooleanLiteral(const Expression* expr, std::ostream& os);
    void GenerateIdentifier(const Expression* expr, std::ostream& os);
    void GenerateBinaryExpression(const Expression* expr, std::ostream& os);
    void GenerateUnaryExpression(const Expression* expr, std::ostream& os);
    void GenerateAssignmentExpression(const Expression* expr, std::ostream& os);
    void GenerateCallExpression(const Expression* expr, std::ostream& os);
    void GenerateMemberExpression(const Expression* expr, std::ostream& os);
    void GenerateArrayExpression(const Expression* expr, std::ostream& os);
    void GenerateObjectExpression(const Expression* expr, std::ostream& os);

    /**
     * @brief 生成各种语句类型
     */
    void GenerateExpressionStatement(const Statement* stmt, std::ostream& os);
    void GenerateBlockStatement(const Statement* stmt, std::ostream& os);
    void GenerateVariableDeclaration(const Statement* stmt, std::ostream& os);
    void GenerateIfStatement(const Statement* stmt, std::ostream& os);
    void GenerateWhileStatement(const Statement* stmt, std::ostream& os);
    void GenerateForStatement(const Statement* stmt, std::ostream& os);
    void GenerateReturnStatement(const Statement* stmt, std::ostream& os);
    void GenerateFunctionDeclaration(const class FunctionExpression* func, std::ostream& os);

    /**
     * @brief 生成函数签名
     */
    void GenerateFunctionSignature(const std::string& name,
                                  const std::vector<std::string>& params,
                                  std::ostream& os);

    /**
     * @brief 获取表达式的类型注释
     */
    std::string GetTypeAnnotation(const Expression* expr);

    CppCodeGeneratorConfig config_;
    TypeInferenceEngine type_engine_;
    NameMangler name_mangler_;
    CodeEmitter emitter_;
    std::string generated_code_;
};

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

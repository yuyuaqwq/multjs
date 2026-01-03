/**
 * @file type_inference_engine.h
 * @brief 类型推断引擎，分析AST推断变量和表达式的类型
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include "./cpp_type.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace mjs {

namespace compiler {

class Expression;
class Statement;
class FunctionExpression;

namespace cpp_gen {

/**
 * @struct FunctionSignature
 * @brief 函数签名，包含参数类型和返回值类型
 */
struct FunctionSignature {
    std::vector<std::shared_ptr<CppType>> param_types;
    std::shared_ptr<CppType> return_type;

    FunctionSignature()
        : return_type(std::make_shared<CppType>(CppType::Value())) {}
};

/**
 * @class TypeInferenceEngine
 * @brief 类型推断引擎
 */
class TypeInferenceEngine {
public:
    /**
     * @brief 构造函数
     */
    TypeInferenceEngine();

    /**
     * @brief 析构函数
     */
    ~TypeInferenceEngine() = default;

    /**
     * @brief 推断表达式类型
     * @param expr 表达式AST节点
     * @return 推断的类型
     */
    std::shared_ptr<CppType> InferExpressionType(const Expression* expr);

    /**
     * @brief 推断语句类型（用于变量声明等）
     * @param stmt 语句AST节点
     */
    void InferStatementType(const Statement* stmt);

    /**
     * @brief 推断函数签名
     * @param func 函数表达式AST节点
     * @return 函数签名
     */
    FunctionSignature InferFunctionSignature(const FunctionExpression* func);

    /**
     * @brief 获取变量类型
     * @param name 变量名
     * @return 变量类型，如果不存在返回nullptr
     */
    std::shared_ptr<CppType> GetVariableType(const std::string& name) const;

    /**
     * @brief 设置变量类型
     * @param name 变量名
     * @param type 变量类型
     */
    void SetVariableType(const std::string& name, std::shared_ptr<CppType> type);

    /**
     * @brief 进入新作用域
     */
    void EnterScope();

    /**
     * @brief 离开当前作用域
     */
    void ExitScope();

    /**
     * @brief 解决类型约束
     * @return 是否成功解决所有约束
     */
    bool SolveConstraints();

    /**
     * @brief 清空所有类型信息
     */
    void Clear();

    /**
     * @brief 获取所有推断出的对象结构体类型
     * @return 对象类型列表（用于生成结构体定义）
     */
    const std::vector<std::shared_ptr<CppType>>& GetObjectTypes() const {
        return object_types_;
    }

private:
    /**
     * @brief 推断二元表达式类型
     */
    std::shared_ptr<CppType> InferBinaryExpressionType(const Expression* expr);

    /**
     * @brief 推断字面量类型
     */
    std::shared_ptr<CppType> InferLiteralType(const Expression* expr);

    /**
     * @brief 推断标识符类型
     */
    std::shared_ptr<CppType> InferIdentifierType(const Expression* expr);

    /**
     * @brief 推断函数调用类型
     */
    std::shared_ptr<CppType> InferCallExpressionType(const Expression* expr);

    /**
     * @brief 推断成员访问类型
     */
    std::shared_ptr<CppType> InferMemberExpressionType(const Expression* expr);

    /**
     * @brief 推断数组表达式类型
     */
    std::shared_ptr<CppType> InferArrayExpressionType(const Expression* expr);

    /**
     * @brief 推断对象表达式类型
     */
    std::shared_ptr<CppType> InferObjectExpressionType(const Expression* expr);

    // 作用域栈
    std::vector<std::unordered_map<std::string, std::shared_ptr<CppType>>> scopes_;

    // 函数签名缓存
    std::unordered_map<std::string, FunctionSignature> function_signatures_;

    // 推断出的对象类型列表（用于生成结构体定义）
    std::vector<std::shared_ptr<CppType>> object_types_;

    // 结构体计数器（用于生成唯一名称）
    int struct_counter_ = 0;
};

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

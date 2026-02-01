/**
 * @file statement_base.h
 * @brief 语句基类定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/class_def/class_def.h>
#include <mjs/token.h>

namespace mjs {

class FunctionDefBase;

namespace compiler {

class Lexer;
class CodeGenerator;

enum class StatementType {
    // 模块相关
    kImport,
    kExport,

    // 声明语句
    kVariableDeclaration,
    kClassDeclaration,

    // 控制流
    kIf,
    kLabeled,

    // 循环及控制
    kFor,
    kWhile,
    kContinue,
    kBreak,

    // 函数控制
    kReturn,

    // 异常处理
    kTry,
    kCatch,
    kFinally,
    kThrow,

    // 基本语句
    kExpression,
    kBlock,

    // 类型
    kTypeAnnotation,
    kPredefinedType,
    kNamedType,
    kLiteralType,
    kUnionType,
    kFunctionType,
};

/**
 * @class Statement
 * @brief 语句基类，所有语句节点的抽象基类
 */
class Statement : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param start 语句起始位置
     * @param end 语句结束位置
     */
    Statement(SourceBytePosition start, SourceBytePosition end)
        : start_(start), end_(end) {}

    /**
     * @brief 析构函数
     */
    virtual ~Statement() = default;

    /**
     * @brief 获取语句类型
     * @return 语句类型
     */
    virtual StatementType type() const noexcept = 0;

    /**
     * @brief 检查语句是否为指定类型
     * @param type 语句类型
     * @return 是否为指定类型
     */
    bool is(StatementType type) const {
        return type == this->type();
    }

    /**
     * @brief 将语句转换为指定类型
     * @tparam T 目标类型
     * @return 转换后的语句引用
     */
    template<typename T>
    T& as() {
        return *static_cast<T*>(this);
    }

    /**
     * @brief 将语句转换为指定类型（常量版本）
     * @tparam T 目标类型
     * @return 转换后的语句常量引用
     */
    template<typename T>
    const T& as() const {
        return *static_cast<const T*>(this);
    }

    /**
     * @brief 获取语句起始位置
     * @return 起始位置
     */
    SourceBytePosition start() const { return start_; }

    /**
     * @brief 获取语句结束位置
     * @return 结束位置
     */
    SourceBytePosition end() const { return end_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    virtual void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const = 0;

    /**
     * @brief 解析语句
     * @param lexer 词法分析器
     * @return 解析后的语句
     */
    static std::unique_ptr<Statement> ParseStatement(Lexer* lexer);

    void current_loop_repair_entries() {  }

private:
    SourceBytePosition start_;
    SourceBytePosition end_;
};

} // namespace compiler
} // namespace mjs
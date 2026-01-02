/**
 * @file arrow_function_expression.h
 * @brief 箭头函数表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../expression.h"

namespace mjs {
namespace compiler {

// 前向声明
class Statement;

/**
 * @class ArrowFunctionExpression
 * @brief 箭头函数表达式
 */
class ArrowFunctionExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param params 函数参数列表
     * @param body 函数体语句
     * @param is_async 是否为异步函数
     */
    ArrowFunctionExpression(SourceBytePosition start, SourceBytePosition end,
        std::vector<std::string>&& params,
        std::unique_ptr<Statement> body,
        bool is_async);

    /**
     * @brief 获取函数参数列表
     * @return 参数列表
     */
    const std::vector<std::string>& params() const { return params_; }

    /**
     * @brief 获取函数体语句
     * @return 函数体语句的常量引用
     */
    const std::unique_ptr<Statement>& body() const { return body_; }

    /**
     * @brief 判断是否为异步函数
     * @return 如果是异步函数则返回true，否则返回false
     */
    bool is_async() const { return is_async_; }

    /**
     * @brief 尝试解析箭头函数表达式
     * @param lexer 词法分析器
     * @param start 源文件位置
     * @param is_async 是否是异步函数
     * @return 返回的表达式
     */
    static std::unique_ptr<Expression> TryParseArrowFunctionExpression(Lexer* lexer, SourceBytePosition start, bool is_async);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::vector<std::string> params_;        ///< 函数参数列表
    std::unique_ptr<Statement> body_;        ///< 函数体语句
    bool is_async_;                          ///< 是否为异步函数
};

} // namespace compiler
} // namespace mjs
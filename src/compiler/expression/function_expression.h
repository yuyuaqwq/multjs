/**
 * @file function_expression.h
 * @brief 函数表达式
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
class BlockStatement;

/**
 * @class FunctionExpression
 * @brief 函数表达式
 */
class FunctionExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param id 函数标识符
     * @param params 函数参数列表
     * @param body 函数体语句
     * @param is_generator 是否为生成器函数
     * @param is_async 是否为异步函数
     * @param is_module 是否为模块函数
     */
    FunctionExpression(SourcePosition start, SourcePosition end,
        std::string id, std::vector<std::string>&& params,
        std::unique_ptr<BlockStatement> body,
        bool is_generator, bool is_async, bool is_module);


    /**
     * @brief 获取函数标识符
     * @return 函数标识符
     */
    const std::string& id() const { return id_; }

    /**
     * @brief 获取函数参数列表
     * @return 参数列表
     */
    const std::vector<std::string>& params() const { return params_; }

    /**
     * @brief 获取函数体语句
     * @return 函数体语句的常量引用
     */
    const std::unique_ptr<BlockStatement>& body() const { return body_; }

    /**
     * @brief 判断是否为生成器函数
     * @return 如果是生成器函数则返回true，否则返回false
     */
    bool is_generator() const { return is_generator_; }

    /**
     * @brief 判断是否为异步函数
     * @return 如果是异步函数则返回true，否则返回false
     */
    bool is_async() const { return is_async_; }

    /**
     * @brief 判断是否为模块函数
     * @return 如果是模块函数则返回true，否则返回false
     */
    bool is_module() const { return is_module_; }

    /**
     * @brief 判断是否为导出函数
     * @return 如果是导出函数则返回true，否则返回false
     */
    bool is_export() const { return is_export_; }

    /**
     * @brief 设置是否为导出函数
     * @param is_export 是否为导出函数
     */
    void set_is_export(bool is_export) { is_export_ = is_export; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析函数表达式
     * @param lexer 词法分析器
     * @return 解析后的表达式
     */
    static std::unique_ptr<Expression> ParseExpressionAtFunctionLevel(Lexer* lexer);

    /**
     * @brief 解析传统函数
     * @param lexer 词法分析器
     * @param start 表达式起始位置
     * @param is_async 是否是异步函数
     * @param is_generator 是否是生成器函数
     * @return 解析后的函数表达式
     */
    static std::unique_ptr<Expression> ParseTraditionalFunctionExpression(Lexer* lexer, SourcePosition start, bool is_async, bool is_generator);

private:
    std::string id_;                         ///< 函数标识符
    std::vector<std::string> params_;        ///< 函数参数列表
    std::unique_ptr<BlockStatement> body_;   ///< 函数体语句

    struct {
        uint32_t is_export_ : 1;    ///< 是否为导出函数
        uint32_t is_generator_ : 1; ///< 是否为生成器函数
        uint32_t is_async_ : 1;     ///< 是否为异步函数
        uint32_t is_module_ : 1;    ///< 是否为模块函数
    };
};

} // namespace compiler
} // namespace mjs
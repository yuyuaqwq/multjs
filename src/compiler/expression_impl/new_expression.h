/**
 * @file new_expression.h
 * @brief new 表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <vector>
#include <memory>
#include "../expression.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class NewExpression
 * @brief new 表达式
 */
class NewExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param callee 构造函数表达式
     * @param arguments 参数表达式列表
     */
    NewExpression(SourcePosition start, SourcePosition end,
                std::unique_ptr<Expression> callee,
                std::vector<std::unique_ptr<Expression>>&& arguments)
        : Expression(start, end), callee_(std::move(callee)),
          arguments_(std::move(arguments)) {}


    /**
     * @brief 获取构造函数表达式
     * @return 构造函数表达式的常量引用
     */
    const std::unique_ptr<Expression>& callee() const { return callee_; }

    /**
     * @brief 获取参数表达式列表
     * @return 参数表达式列表的常量引用
     */
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析new表达式
     * @param lexer 词法分析器
     * @return 解析后的new表达式
     */
    static std::unique_ptr<Expression> ParseNewExpression(Lexer* lexer);

private:
    std::unique_ptr<Expression> callee_; ///< 构造函数表达式
    std::vector<std::unique_ptr<Expression>> arguments_; ///< 参数表达式列表
};

} // namespace compiler
} // namespace mjs
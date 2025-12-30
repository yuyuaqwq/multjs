/**
 * @file yield_expression.h
 * @brief yield 表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>
#include "../expression.h"
#include "../lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class YieldExpression
 * @brief yield 表达式
 */
class YieldExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param argument yield 参数表达式
     * @param is_delegate 是否为委托yield (yield*)
     */
    YieldExpression(SourcePosition start, SourcePosition end,
                std::unique_ptr<Expression> argument, bool is_delegate = false)
        : Expression(start, end), argument_(std::move(argument)), is_delegate_(is_delegate) {}


    /**
     * @brief 获取 yield 参数表达式
     * @return yield 参数表达式的常量引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }

    /**
     * @brief 获取是否为委托yield
     * @return 是否为委托yield
     */
    bool is_delegate() const { return is_delegate_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

    /**
     * @brief 解析yield优先级的表达式
     * @param lexer 词法分析器
     * @return 表达式指针
     */
    static std::unique_ptr<Expression> ParseExpressionAtYieldLevel(Lexer* lexer);

    /**
     * @brief 解析yield表达式
     * @param lexer 词法分析器
     * @return yield表达式指针
     */
    static std::unique_ptr<YieldExpression> ParseYieldExpression(Lexer* lexer);

private:
    std::unique_ptr<Expression> argument_; ///< yield 参数表达式
    bool is_delegate_; ///< 是否为委托yield (yield*)
};

} // namespace compiler
} // namespace mjs
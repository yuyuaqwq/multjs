/**
 * @file await_expression.h
 * @brief await 表达式
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>
#include "../expression.h"

namespace mjs {
namespace compiler {

/**
 * @class AwaitExpression
 * @brief await 表达式
 */
class AwaitExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param argument await 参数表达式
     */
    AwaitExpression(SourceBytePosition start, SourceBytePosition end,
        std::unique_ptr<Expression> argument)
        : Expression(start, end), argument_(std::move(argument)) {}


    /**
     * @brief 获取 await 参数表达式
     * @return await 参数表达式的常量引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> argument_; ///< await 参数表达式
};

} // namespace compiler
} // namespace mjs
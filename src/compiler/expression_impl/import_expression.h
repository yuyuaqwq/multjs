/**
 * @file import_expression.h
 * @brief import 表达式
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
 * @class ImportExpression
 * @brief import 表达式
 */
class ImportExpression : public Expression {
public:
    /**
     * @brief 构造函数
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param source 导入源表达式
     */
    ImportExpression(SourceBytePosition start, SourceBytePosition end, std::unique_ptr<Expression> source)
        : Expression(start, end), source_(std::move(source)) {}


    /**
     * @brief 获取导入源表达式
     * @return 导入源表达式的常量引用
     */
    const std::unique_ptr<Expression>& source() const { return source_; }

    static std::unique_ptr<ImportExpression> ParseImportExpression(Lexer* lexer);

    /**
     * @brief 生成代码
     * @param code_generator 代码生成器
     * @param function_def_base 函数定义
     */
    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Expression> source_; ///< 导入源表达式
};

} // namespace compiler
} // namespace mjs
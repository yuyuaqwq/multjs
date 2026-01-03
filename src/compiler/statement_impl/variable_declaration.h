/**
 * @file variable_declaration.h
 * @brief 变量声明定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>

#include "src/compiler/statement.h"
#include "src/compiler/expression.h"
#include "src/compiler/lexer.h"

namespace mjs {
namespace compiler {

/**
 * @class VariableDeclaration
 * @brief 变量声明语句
 */
class VariableDeclaration : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param name 变量名
     * @param init 初始化表达式
     * @param kind 变量类型
     */
    VariableDeclaration(SourceBytePosition start, SourceBytePosition end,
        std::string name,
        std::unique_ptr<Expression> init,
        TokenType kind)
        : Statement(start, end),
        name_(std::move(name)),
        init_(std::move(init)),
        kind_(kind) {
        is_export_ = false;
    }

    StatementType type() const noexcept override { return StatementType::kVariableDeclaration; }

    /**
     * @brief 获取变量名
     * @return 变量名引用
     */
    const std::string& name() const { return name_; }

    /**
     * @brief 获取初始化表达式
     * @return 初始化表达式引用
     */
    const std::unique_ptr<Expression>& init() const { return init_; }

    /**
     * @brief 获取变量类型
     * @return 变量类型
     */
    TokenType kind() const { return kind_; }

    /**
     * @brief 检查是否为导出
     * @return 是否为导出
     */
    bool is_export() const { return is_export_; }

    /**
     * @brief 设置是否为导出
     * @param is_export 是否为导出
     */
    void set_is_export(bool is_export) { is_export_ = is_export; }

    /**
     * @brief 解析变量声明
     * @param lexer 词法分析器
     * @param kind 变量类型
     * @return 解析后的变量声明
     */
    static std::unique_ptr<VariableDeclaration> ParseVariableDeclaration(Lexer* lexer, TokenType kind);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string name_;
    std::unique_ptr<Expression> init_;
    TokenType kind_;

    struct {
        uint32_t is_export_ : 1;
    };
};

} // namespace compiler
} // namespace mjs
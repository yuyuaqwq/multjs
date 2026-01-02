/**
 * @file export_declaration.h
 * @brief 导出声明定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <memory>

#include "../statement.h"

namespace mjs {
namespace compiler {

/**
 * @class ExportDeclaration
 * @brief 导出声明语句
 */
class ExportDeclaration : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param declaration 被导出的声明
     */
    explicit ExportDeclaration(SourceBytePosition start, SourceBytePosition end,
        std::unique_ptr<Statement> declaration)
        : Statement(start, end), declaration_(std::move(declaration)) {}

    StatementType type() const noexcept override { return StatementType::kExport; }

    /**
     * @brief 获取被导出的声明
     * @return 声明引用
     */
    const std::unique_ptr<Statement>& declaration() const { return declaration_; }

	/**
	 * @brief 解析export声明
	 * @param type 标记类型
	 * @return 解析后的export声明
	 */
	static std::unique_ptr<ExportDeclaration> ParseExportDeclaration(Lexer* lexer, TokenType type);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::unique_ptr<Statement> declaration_;
};

} // namespace compiler
} // namespace mjs
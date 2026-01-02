/**
 * @file import_declaration.h
 * @brief 导入声明定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>

#include "../statement.h"

namespace mjs {
namespace compiler {

/**
 * @class ImportDeclaration
 * @brief 导入声明语句
 */
class ImportDeclaration : public Statement {
public:
    /**
     * @brief 构造函数
     * @param start 起始位置
     * @param end 结束位置
     * @param source 导入源
     * @param name 导入名称
     */
    ImportDeclaration(SourceBytePosition start, SourceBytePosition end,
        std::string source,
        std::string name)
        : Statement(start, end),
        source_(std::move(source)),
        name_(std::move(name)) {}

    StatementType type() const noexcept override { return StatementType::kImport; }

    /**
     * @brief 获取导入源
     * @return 导入源引用
     */
    const std::string& source() const { return source_; }

    /**
     * @brief 获取导入名称
     * @return 导入名称引用
     */
    const std::string& name() const { return name_; }

	/**
	 * @brief 解析import语句
	 * @param type 标记类型
	 * @return 解析后的import语句
	 */
	static std::unique_ptr<Statement> ParseImportStatement(Lexer* lexer, TokenType type);

    void GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const override;

private:
    std::string source_;
    std::string name_;
};

} // namespace compiler
} // namespace mjs
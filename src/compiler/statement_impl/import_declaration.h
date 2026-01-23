/**
 * @file import_declaration.h
 * @brief 导入声明定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#pragma once

#include <string>
#include <vector>

#include "src/compiler/statement.h"

namespace mjs {
namespace compiler {

/**
 * @struct ImportSpecifier
 * @brief 导入说明符，用于命名导入
 */
struct ImportSpecifier {
    std::string imported_name;  ///< 导入的导出名称
    std::string local_name;     ///< 本地绑定名称

    ImportSpecifier(std::string imported, std::string local)
        : imported_name(std::move(imported)), local_name(std::move(local)) {}
};

/**
 * @class ImportDeclaration
 * @brief 导入声明语句
 */
class ImportDeclaration : public Statement {
public:
    /**
     * @brief 构造函数（默认导入）
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

    /**
     * @brief 构造函数（命名导入）
     * @param start 起始位置
     * @param end 结束位置
     * @param source 导入源
     * @param specifiers 导入说明符列表
     */
    ImportDeclaration(SourceBytePosition start, SourceBytePosition end,
        std::string source,
        std::vector<ImportSpecifier> specifiers)
        : Statement(start, end),
          source_(std::move(source)),
          specifiers_(std::move(specifiers)) {}

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
     * @brief 获取导入说明符列表
     * @return 导入说明符列表引用
     */
    const std::vector<ImportSpecifier>& specifiers() const { return specifiers_; }

    /**
     * @brief 是否为命名导入
     * @return 如果是命名导入返回 true
     */
    bool is_named_import() const { return !specifiers_.empty(); }

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
    std::vector<ImportSpecifier> specifiers_;
};

} // namespace compiler
} // namespace mjs
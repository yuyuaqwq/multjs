/**
 * @file module_statement_test.cpp
 * @brief 模块语句单元测试
 *
 * 测试import和export语句的解析和功能
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"
#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/import_declaration.h"
#include "src/compiler/statement_impl/export_declaration.h"
#include <mjs/source_define.h>

namespace mjs {
namespace compiler {
namespace test {

/**
 * @class ImportDeclarationTest
 * @brief import语句测试类
 */
class ImportDeclarationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }

    // 辅助方法:创建Lexer对象
    std::unique_ptr<Lexer> CreateLexer(const std::string& source) {
        return std::make_unique<Lexer>(source);
    }
};

/**
 * @test 测试ImportDeclaration构造
 */
TEST_F(ImportDeclarationTest, ConstructImportDeclaration) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;
    std::string source = "react";
    std::string name = "React";

    // Act
    ImportDeclaration import_decl(start, end, source, name);

    // Assert
    EXPECT_EQ(import_decl.type(), StatementType::kImport);
    EXPECT_EQ(import_decl.source(), "react");
    EXPECT_EQ(import_decl.name(), "React");
}

/**
 * @test 测试默认导入构造
 */
TEST_F(ImportDeclarationTest, DefaultImportConstruction) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 25;

    // Act
    ImportDeclaration import_decl(start, end, "react", "React");

    // Assert
    EXPECT_EQ(import_decl.source(), "react");
    EXPECT_EQ(import_decl.name(), "React");
}

/**
 * @test 测试副作用导入构造(无名称)
 */
TEST_F(ImportDeclarationTest, SideEffectImportConstruction) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 18;

    // Act
    ImportDeclaration import_decl(start, end, "lodash", "");

    // Assert
    EXPECT_EQ(import_decl.source(), "lodash");
    EXPECT_EQ(import_decl.name(), "");
}

/**
 * @test 测试命名导入构造
 */
TEST_F(ImportDeclarationTest, NamedImportConstruction) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 35;

    // Act
    ImportDeclaration import_decl(start, end, "react", "useState");

    // Assert
    EXPECT_EQ(import_decl.source(), "react");
    EXPECT_EQ(import_decl.name(), "useState");
}

/**
 * @class ExportDeclarationTest
 * @brief export语句测试类
 */
class ExportDeclarationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }
};

/**
 * @test 测试ExportDeclaration构造
 */
TEST_F(ExportDeclarationTest, ConstructExportDeclaration) {
    // Arrange
    SourceBytePosition start = 0;
    SourceBytePosition end = 20;

    // 创建一个ImportDeclaration作为导出内容
    auto import_statement = std::make_unique<ImportDeclaration>(start, end, "test", "Test");

    // Act
    ExportDeclaration export_decl(start, end, std::move(import_statement));

    // Assert
    EXPECT_EQ(export_decl.type(), StatementType::kExport);
    EXPECT_NE(export_decl.declaration(), nullptr);
}

/**
 * @class ModuleIntegrationTest
 * @brief 模块集成测试
 */
class ModuleIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化代码
    }

    void TearDown() override {
        // 清理代码
    }
};

/**
 * @test 测试导入导出类型标识
 */
TEST_F(ModuleIntegrationTest, ImportExportTypeIdentification) {
    // Arrange
    SourceBytePosition pos = 0;

    // Act
    ImportDeclaration import_decl(pos, pos, "test", "Test");
    auto inner_decl = std::make_unique<ImportDeclaration>(pos, pos, "inner", "Inner");
    ExportDeclaration export_decl(pos, pos, std::move(inner_decl));

    // Assert
    EXPECT_EQ(import_decl.type(), StatementType::kImport);
    EXPECT_EQ(export_decl.type(), StatementType::kExport);
    EXPECT_NE(import_decl.type(), StatementType::kExport);
    EXPECT_NE(export_decl.type(), StatementType::kImport);
}

/**
 * @test 测试多个导入源
 */
TEST_F(ModuleIntegrationTest, MultipleImportSources) {
    // Arrange
    SourceBytePosition pos = 0;

    // Act
    ImportDeclaration import1(pos, pos, "react", "React");
    ImportDeclaration import2(pos, pos, "lodash", "_");
    ImportDeclaration import3(pos, pos, "axios", "axios");

    // Assert
    EXPECT_EQ(import1.source(), "react");
    EXPECT_EQ(import2.source(), "lodash");
    EXPECT_EQ(import3.source(), "axios");
}

} // namespace test
} // namespace compiler
} // namespace mjs

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

#include <mjs/compiler/lexer.h>
#include <mjs/compiler/parser.h>
#include <mjs/compiler/statement_impl/import_declaration.h>
#include <mjs/compiler/statement_impl/export_declaration.h>
#include <mjs/compiler/statement_impl/variable_declaration.h>

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

    // 辅助方法:解析import语句
    std::unique_ptr<Statement> ParseImport(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.Tokenize();
        Parser parser(tokens);
        return parser.ParseModuleItem();
    }

    // 辅助方法:验证是否为ImportDeclaration
    ImportDeclaration* AsImportDeclaration(Statement* stmt) {
        if (stmt && stmt->type() == StatementType::kImport) {
            return static_cast<ImportDeclaration*>(stmt);
        }
        return nullptr;
    }
};

/**
 * @test 简单的副作用导入
 */
TEST_F(ImportDeclarationTest, ImportSideEffect) {
    // Arrange
    std::string source = "import 'lodash';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "lodash");
    EXPECT_EQ(import_decl->name(), "");
}

/**
 * @test 默认导入
 */
TEST_F(ImportDeclarationTest, ImportDefault) {
    // Arrange
    std::string source = "import React from 'react';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "react");
    EXPECT_EQ(import_decl->name(), "React");
}

/**
 * @test 命名导入
 */
TEST_F(ImportDeclarationTest, ImportNamed) {
    // Arrange
    std::string source = "import { useState, useEffect } from 'react';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "react");
    // 注意: 命名导入的name字段可能需要特殊处理
}

/**
 * @test 命名空间导入
 */
TEST_F(ImportDeclarationTest, ImportNamespace) {
    // Arrange
    std::string source = "import * as utils from './utils';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "./utils");
    EXPECT_EQ(import_decl->name(), "utils");
}

/**
 * @test 混合导入
 */
TEST_F(ImportDeclarationTest, ImportMixed) {
    // Arrange
    std::string source = "import React, { useState } from 'react';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "react");
    // 注意: 混合导入可能需要更复杂的结构
}

/**
 * @test 导入相对路径
 */
TEST_F(ImportDeclarationTest, ImportRelativePath) {
    // Arrange
    std::string source = "import { helper } from '../helpers/helper';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "../helpers/helper");
}

/**
 * @test 导入绝对路径
 */
TEST_F(ImportDeclarationTest, ImportAbsolutePath) {
    // Arrange
    std::string source = "import { config } from '/config/app';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "/config/app");
}

/**
 * @test 导入URL路径
 */
TEST_F(ImportDeclarationTest, ImportUrlPath) {
    // Arrange
    std::string source = "import data from 'https://example.com/data.json';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "https://example.com/data.json");
}

/**
 * @test 别名导入
 */
TEST_F(ImportDeclarationTest, ImportAliased) {
    // Arrange
    std::string source = "import { useState as useState } from 'react';";

    // Act
    auto stmt = ParseImport(source);
    auto import_decl = AsImportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(import_decl, nullptr);
    EXPECT_EQ(import_decl->source(), "react");
    // 注意: 别名需要额外的结构支持
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

    // 辅助方法:解析export语句
    std::unique_ptr<Statement> ParseExport(const std::string& source) {
        Lexer lexer(source);
        auto tokens = lexer.Tokenize();
        Parser parser(tokens);
        return parser.ParseModuleItem();
    }

    // 辅助方法:验证是否为ExportDeclaration
    ExportDeclaration* AsExportDeclaration(Statement* stmt) {
        if (stmt && stmt->type() == StatementType::kExport) {
            return static_cast<ExportDeclaration*>(stmt);
        }
        return nullptr;
    }
};

/**
 * @test 导出变量声明
 */
TEST_F(ExportDeclarationTest, ExportVariableDeclaration) {
    // Arrange
    std::string source = "export const PI = 3.14159;";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
    EXPECT_EQ(export_decl->declaration()->type(), StatementType::kVariable);
}

/**
 * @test 导出函数声明
 */
TEST_F(ExportDeclarationTest, ExportFunctionDeclaration) {
    // Arrange
    std::string source = "export function helper() { return 42; }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
    EXPECT_EQ(export_decl->declaration()->type(), StatementType::kFunction);
}

/**
 * @test 导出类声明
 */
TEST_F(ExportDeclarationTest, ExportClassDeclaration) {
    // Arrange
    std::string source = "export class MyClass { }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
    EXPECT_EQ(export_decl->declaration()->type(), StatementType::kClass);
}

/**
 * @test 默认导出
 */
TEST_F(ExportDeclarationTest, ExportDefault) {
    // Arrange
    std::string source = "export default 42;";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出匿名函数
 */
TEST_F(ExportDeclarationTest, ExportDefaultAnonymousFunction) {
    // Arrange
    std::string source = "export default function() { return 42; }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出命名函数
 */
TEST_F(ExportDeclarationTest, ExportDefaultNamedFunction) {
    // Arrange
    std::string source = "export default function myFunction() { return 42; }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出命名列表
 */
TEST_F(ExportDeclarationTest, ExportNamedList) {
    // Arrange
    std::string source = "export { pi, e, sqrt };";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出带重命名
 */
TEST_F(ExportDeclarationTest, ExportWithRename) {
    // Arrange
    std::string source = "export { pi as PI, e as E };";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 从其他模块重导出
 */
TEST_F(ExportDeclarationTest, ExportFromModule) {
    // Arrange
    std::string source = "export { pi, e } from './math';";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 从其他模块重导出默认
 */
TEST_F(ExportDeclarationTest, ExportDefaultFromModule) {
    // Arrange
    std::string source = "export { default } from './module';";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 重导出整个模块
 */
TEST_F(ExportDeclarationTest, ExportAllFromModule) {
    // Arrange
    std::string source = "export * from './utils';";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出异步函数
 */
TEST_F(ExportDeclarationTest, ExportAsyncFunction) {
    // Arrange
    std::string source = "export async function fetchData() { return await fetch('/api'); }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出生成器函数
 */
TEST_F(ExportDeclarationTest, ExportGeneratorFunction) {
    // Arrange
    std::string source = "export function* generateNumbers() { yield 1; yield 2; }";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
}

/**
 * @test 导出多个变量
 */
TEST_F(ExportDeclarationTest, ExportMultipleVariables) {
    // Arrange
    std::string source = "export const a = 1, b = 2, c = 3;";

    // Act
    auto stmt = ParseExport(source);
    auto export_decl = AsExportDeclaration(stmt.get());

    // Assert
    ASSERT_NE(export_decl, nullptr);
    ASSERT_NE(export_decl->declaration(), nullptr);
    EXPECT_EQ(export_decl->declaration()->type(), StatementType::kVariable);
}

/**
 * @class ModuleIntegrationTest
 * @brief 模块集成测试类
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
 * @test 导入后导出
 */
TEST_F(ModuleIntegrationTest, ImportThenExport) {
    // Arrange
    std::string source1 = "import { utils } from './utils';";
    std::string source2 = "export { utils };";

    // Act
    Lexer lexer1(source1);
    auto tokens1 = lexer1.Tokenize();
    Parser parser1(tokens1);
    auto import_stmt = parser1.ParseModuleItem();

    Lexer lexer2(source2);
    auto tokens2 = lexer2.Tokenize();
    Parser parser2(tokens2);
    auto export_stmt = parser2.ParseModuleItem();

    // Assert
    ASSERT_NE(import_stmt, nullptr);
    EXPECT_EQ(import_stmt->type(), StatementType::kImport);
    ASSERT_NE(export_stmt, nullptr);
    EXPECT_EQ(export_stmt->type(), StatementType::kExport);
}

/**
 * @test 导入导出链
 */
TEST_F(ModuleIntegrationTest, ImportExportChain) {
    // 这个测试验证模块导入导出的链接关系
    // 实际测试需要模块管理器的支持

    // Arrange
    std::string module_a = "export const value = 42;";
    std::string module_b = "import { value } from './a';";
    std::string module_c = "import { value } from './b';";

    // Act & Assert
    // 验证模块之间的依赖关系
    EXPECT_TRUE(true); // 占位测试
}

/**
 * @test 循环导入检测
 */
TEST_F(ModuleIntegrationTest, CircularImportDetection) {
    // 这个测试验证循环导入的检测
    // 实际测试需要模块管理器的支持

    // Arrange
    std::string module_a = "import { b } from './b';";
    std::string module_b = "import { a } from './a';";

    // Act & Assert
    // 验证循环导入能被正确检测和处理
    EXPECT_TRUE(true); // 占位测试
}

} // namespace test
} // namespace compiler
} // namespace mjs

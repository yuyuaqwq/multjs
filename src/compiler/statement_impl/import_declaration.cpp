#include "src/compiler/statement_impl/import_declaration.h"

#include <mjs/error.h>

#include "src/compiler/code_generator.h"
#include "src/compiler/statement_impl/expression_statement.h"

namespace mjs {
namespace compiler {

void ImportDeclaration::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    if (is_named_import()) {
        // 命名导入: import { foo, bar as baz } from 'module'
        auto source_const_idx = code_generator->AllocateConst(Value(String::New(source_)));
        function_def_base->bytecode_table().EmitConstLoad(source_const_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetModule);

        // 为每个导入的导出创建局部变量绑定
        for (const auto& spec : specifiers_) {
            // 复制模块对象（因为每次属性访问都需要对象）
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kDump);

            // 获取模块对象的导出属性
            auto prop_const_idx = code_generator->AllocateConst(Value(String::New(spec.imported_name)));
            function_def_base->bytecode_table().EmitPropertyLoad(prop_const_idx);

            // 存储到局部变量
            auto& var_info = code_generator->scope_manager().AllocateVar(spec.local_name, VarFlags::kConst);
            function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);

			// pop掉导出属性
			function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
        }

        // 清理栈上的模块对象
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
    } else {
        // 默认导入或整体导入: import foo from 'module' 或 import * as foo from 'module'
        auto source_const_idx = code_generator->AllocateConst(Value(String::New(source_)));
        function_def_base->bytecode_table().EmitConstLoad(source_const_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetModule);

        if (!name_.empty()) {
            // 模块对象保存到变量
            auto& var_info = code_generator->scope_manager().AllocateVar(name_, VarFlags::kConst);
            function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
        }
    }
}

std::unique_ptr<Statement> ImportDeclaration::ParseImportStatement(Lexer* lexer, TokenType type) {
	auto start = lexer->GetSourcePosition();
	auto token = lexer->PeekTokenN(2);
	if (token.is(TokenType::kOpMul)) {
		// 整体导入: import * as foo from 'module'
		lexer->MatchToken(type);
		auto token = lexer->NextToken();
		lexer->MatchToken(TokenType::kKwAs);
		auto module_name = lexer->MatchToken(TokenType::kIdentifier).value();
		lexer->MatchToken(TokenType::kKwFrom);

		auto source = lexer->MatchToken(TokenType::kString).value();

		lexer->MatchToken(TokenType::kSepSemi);

		auto end = lexer->GetRawSourcePosition();

		auto import_declaration = std::make_unique<ImportDeclaration>(start, end, std::move(source), std::move(module_name));
		return import_declaration;
	}
	else if (token.is(TokenType::kSepLParen)) {
		// 动态import
		return ExpressionStatement::ParseExpressionStatement(lexer);
	}
	else if (token.is(TokenType::kSepLCurly)) {
		// 命名导入: import { foo, bar as baz } from 'module'
		lexer->MatchToken(type);
		lexer->MatchToken(TokenType::kSepLCurly);

		// 解析导入的标识符列表
		std::vector<ImportSpecifier> specifiers;
		while (!lexer->PeekToken().is(TokenType::kSepRCurly)) {
			auto imported_name = lexer->MatchToken(TokenType::kIdentifier).value();
			std::string local_name = imported_name;

			if (lexer->PeekToken().is(TokenType::kKwAs)) {
				lexer->NextToken();
				local_name = lexer->MatchToken(TokenType::kIdentifier).value();
			}

			specifiers.emplace_back(std::move(imported_name), std::move(local_name));

			if (lexer->PeekToken().is(TokenType::kSepComma)) {
				lexer->NextToken();
			}
		}
		lexer->MatchToken(TokenType::kSepRCurly);
		lexer->MatchToken(TokenType::kKwFrom);

		auto source = lexer->MatchToken(TokenType::kString).value();
		lexer->MatchToken(TokenType::kSepSemi);

		auto end = lexer->GetRawSourcePosition();

		auto import_declaration = std::make_unique<ImportDeclaration>(start, end, std::move(source), std::move(specifiers));
		return import_declaration;
	}
	else {
		throw SyntaxError("Unsupported module parsing.");
	}
}

} // namespace compiler
} // namespace mjs

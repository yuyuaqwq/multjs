#include "src/compiler/statement_impl/import_declaration.h"

#include <mjs/error.h>

#include "src/compiler/code_generator.h"
#include "src/compiler/statement_impl/expression_statement.h"

namespace mjs {
namespace compiler {

void ImportDeclaration::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto source_const_idx = code_generator->AllocateConst(Value(String::New(source_)));

    function_def_base->bytecode_table().EmitConstLoad(source_const_idx);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGetModule);

    auto name_const_idx = code_generator->AllocateConst(Value(String::New(name_)));

    // 模块对象保存到变量
    auto& var_info = code_generator->scope_manager().AllocateVar(name_, VarFlags::kConst);
    function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

std::unique_ptr<Statement> ImportDeclaration::ParseImportStatement(Lexer* lexer, TokenType type) {
	auto start = lexer->GetSourcePosition();
	auto token = lexer->PeekTokenN(2);
	if (token.is(TokenType::kOpMul)) {
		lexer->MatchToken(type);
		auto token = lexer->NextToken();
		lexer->MatchToken(TokenType::kKwAs);
		auto module_name = lexer->MatchToken(TokenType::kIdentifier).value();
		lexer->MatchToken(TokenType::kKwFrom);

		auto source = lexer->MatchToken(TokenType::kString).value();

		lexer->MatchToken(TokenType::kSepSemi);

		// 静态import会被提升，单独保存
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
		while (!lexer->PeekToken().is(TokenType::kSepRCurly)) {
			lexer->MatchToken(TokenType::kIdentifier);
			if (lexer->PeekToken().is(TokenType::kKwAs)) {
				lexer->NextToken();
				lexer->MatchToken(TokenType::kIdentifier);
			}
			if (lexer->PeekToken().is(TokenType::kSepComma)) {
				lexer->NextToken();
			}
		}
		lexer->MatchToken(TokenType::kSepRCurly);
		lexer->MatchToken(TokenType::kKwFrom);

		auto source = lexer->MatchToken(TokenType::kString).value();
		lexer->MatchToken(TokenType::kSepSemi);

		auto end = lexer->GetRawSourcePosition();

		// 命名导入，使用空字符串作为name
		auto import_declaration = std::make_unique<ImportDeclaration>(start, end, std::move(source), "");
		return import_declaration;
	}
	else {
		throw SyntaxError("Unsupported module parsing.");
	}
}

} // namespace compiler
} // namespace mjs
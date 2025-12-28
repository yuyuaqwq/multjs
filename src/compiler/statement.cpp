#include "statement.h"

#include "statement/import_declaration.h"
#include "statement/export_declaration.h"
#include "statement/variable_declaration.h"
#include "statement/if_statement.h"
#include "statement/for_statement.h"
#include "statement/while_statement.h"
#include "statement/continue_statement.h"
#include "statement/break_statement.h"
#include "statement/return_statement.h"
#include "statement/throw_statement.h"
#include "statement/try_statement.h"
#include "statement/block_statement.h"
#include "statement/labeled_statement.h"
#include "statement/expression_statement.h"
#include "expression/function_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<Statement> Statement::ParseStatement(Lexer* lexer) {
	auto token = lexer->PeekToken();
	switch (token.type()) {
	case TokenType::kKwImport: {
		return ImportDeclaration::ParseImportStatement(lexer, token.type());
	}
	case TokenType::kKwExport: {
		return ExportDeclaration::ParseExportDeclaration(lexer, token.type());
	}

	case TokenType::kKwLet:
	case TokenType::kKwConst: {
		return VariableDeclaration::ParseVariableDeclaration(lexer, token.type());
	}

	case TokenType::kKwIf: {
		return IfStatement::ParseIfStatement(lexer);
	}
	case TokenType::kIdentifier: {
		if (lexer->PeekTokenN(2).is(TokenType::kSepColon)) {
			return LabeledStatement::ParseLabeledStatement(lexer);
		}
		return ExpressionStatement::ParseExpressionStatement(lexer);
	}

	case TokenType::kKwFor: {
		return ForStatement::ParseForStatement(lexer);
	}
	case TokenType::kKwWhile: {
		return WhileStatement::ParseWhileStatement(lexer);
	}
	case TokenType::kKwContinue: {
		return ContinueStatement::ParseContinueStatement(lexer);
	}
	case TokenType::kKwBreak: {
		return BreakStatement::ParseBreakStatement(lexer);
	}

	case TokenType::kKwAsync:
	case TokenType::kKwFunction: {
		// 如果是直接定义，就不需要添加分号
		auto start = lexer->GetSourcePosition();
		auto exp = FunctionExpression::ParseExpressionAtFunctionLevel(lexer);
		auto end = lexer->GetRawSourcePosition();
		return std::make_unique<ExpressionStatement>(start, end, std::move(exp));
	}
	case TokenType::kKwReturn: {
		return ReturnStatement::ParseReturnStatement(lexer);
	}

	case TokenType::kKwThrow: {
		return ThrowStatement::ParseThrowStatement(lexer);
	}
	case TokenType::kKwTry: {
		return TryStatement::ParseTryStatement(lexer);
	}

	case TokenType::kSepLCurly: {
		return BlockStatement::ParseBlockStatement(lexer);
	}
	default: {
		return ExpressionStatement::ParseExpressionStatement(lexer);
	}
	}
}

} // namespace compiler
} // namespace mjs
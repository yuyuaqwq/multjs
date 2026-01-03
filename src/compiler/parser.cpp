#include "src/compiler/parser.h"

#include <unordered_set>
#include <stdexcept>

#include <mjs/error.h>

#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/named_type.h"

#include "src/compiler/expression_impl/function_expression.h"
#include "src/compiler/expression_impl/yield_expression.h"
#include "src/compiler/statement_impl/union_type.h"

/* EBNF
exp = exp3
exp3 = exp2 {oper3 exp2}
oper3 = '==' | '!='
exp2 = exp1 {oper2 exp1}
oper2 = '+' | '-'
exp1 = exp0 {oper1 exp0}
oper1 = '*' | '/'
exp0 = '(' exp ')' | exp0
exp0 = number
*/

namespace mjs {
namespace compiler {

Parser::Parser(Lexer* lexer)
	: lexer_(lexer) {}

void Parser::ParseProgram() {
	while (!lexer_->PeekToken().is(TokenType::kEof)) {
		auto statement = Statement::ParseStatement(lexer_);
		if (statement->is(StatementType::kImport)) {
			auto import_declaration = std::unique_ptr<ImportDeclaration>(&statement.release()->as<ImportDeclaration>());
			import_declarations_.emplace_back(std::move(import_declaration));
		}
		else {
			statements_.emplace_back(std::move(statement));
		}
	}
}

std::vector<std::string> Parser::ParseParameters() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->PeekToken().is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).value());

			TypeAnnotation::TryParseTypeAnnotation(lexer_);

			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::vector<std::unique_ptr<Expression>> Parser::ParseExpressions(
	TokenType begin, TokenType end, bool allow_comma_end) {

	lexer_->MatchToken(begin);
	std::vector<std::unique_ptr<Expression>> par_list;
	if (!lexer_->PeekToken().is(end)) {
		do {
			// 避免解析kSepComma
			par_list.emplace_back(YieldExpression::ParseExpressionAtYieldLevel(lexer_));
			if (!lexer_->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
			if (allow_comma_end && lexer_->PeekToken().is(end)) {
				break;
			}
		} while (true);
	}
	lexer_->MatchToken(end);
	return par_list;
}

} // namespace compiler
} // namespace mjs
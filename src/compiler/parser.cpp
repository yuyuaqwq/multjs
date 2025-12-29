#include "./parser.h"

#include <unordered_set>
#include <stdexcept>

#include <mjs/error.h>

#include "statement.h"
#include "statement/named_type.h"

#include "expression/function_expression.h"
#include "expression/yield_expression.h"

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

			TryParseTypeAnnotation();

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

std::unique_ptr<TypeAnnotation> Parser::TryParseTypeAnnotation() {
	if (!lexer_->PeekToken().is(TokenType::kSepColon)) {
		return nullptr;
	}

	auto start = lexer_->GetSourcePosition();
	lexer_->MatchToken(TokenType::kSepColon);

	// 解析类型
	std::unique_ptr<Type> type;

	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		// 命名类型
		auto type_start = lexer_->GetSourcePosition();
		auto type_name = lexer_->NextToken().value();
		auto type_end = lexer_->GetRawSourcePosition();

		type = std::make_unique<NamedType>(type_start, type_end, std::move(type_name));
	} else if (lexer_->PeekToken().is(TokenType::kSepLParen)) {
		// 联合类型
		type = ParseUnionType();
	} else {
		throw SyntaxError("Invalid type annotation");
	}

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<TypeAnnotation>(start, end, std::move(type));
}

std::unique_ptr<UnionType> Parser::ParseUnionType() {
	auto start = lexer_->GetSourcePosition();

	// 解析联合类型的成员
	std::vector<std::unique_ptr<Type>> types;

	// 第一个类型
	if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
		auto type_start = lexer_->GetSourcePosition();
		auto type_name = lexer_->NextToken().value();
		auto type_end = lexer_->GetRawSourcePosition();

		types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
	} else {
		throw SyntaxError("Expected type name");
	}

	// 后续类型
	while (lexer_->PeekToken().is(TokenType::kOpBitOr)) {
		lexer_->NextToken(); // 消耗 |

		if (lexer_->PeekToken().is(TokenType::kIdentifier)) {
			auto type_start = lexer_->GetSourcePosition();
			auto type_name = lexer_->NextToken().value();
			auto type_end = lexer_->GetRawSourcePosition();

			types.push_back(std::make_unique<NamedType>(type_start, type_end, std::move(type_name)));
		} else {
			throw SyntaxError("Expected type name after |");
		}
	}

	auto end = lexer_->GetRawSourcePosition();
	return std::make_unique<UnionType>(start, end, std::move(types));
}


} // namespace compiler
} // namespace mjs
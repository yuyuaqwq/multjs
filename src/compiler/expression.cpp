#include "expression.h"

#include "lexer.h"
#include "expression/binary_expression.h"
#include "expression/assignment_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<Expression> Expression::ParseExpression(Lexer* lexer) {
	return BinaryExpression::ParseExpressionAtCommaLevel(lexer);
}

std::vector<std::string> Expression::ParseParameters(Lexer* lexer) {
	lexer->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer->PeekToken().is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer->MatchToken(TokenType::kIdentifier).value());

			// 类型注解暂时跳过
			if (lexer->PeekToken().is(TokenType::kSepColon)) {
				lexer->NextToken();
				if (lexer->PeekToken().is(TokenType::kIdentifier)) {
					lexer->NextToken();
				}
			}

			if (!lexer->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer->NextToken();
		} while (true);
	}
	lexer->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::vector<std::unique_ptr<Expression>> Expression::ParseExpressions(
	Lexer* lexer, TokenType begin, TokenType end, bool allow_comma_end) {

	lexer->MatchToken(begin);
	std::vector<std::unique_ptr<Expression>> par_list;
	if (!lexer->PeekToken().is(end)) {
		do {
			// 避免解析kSepComma
			par_list.emplace_back(AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer));
			if (!lexer->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer->NextToken();
			if (allow_comma_end && lexer->PeekToken().is(end)) {
				break;
			}
		} while (true);
	}
	lexer->MatchToken(end);
	return par_list;
}

} // namespace compiler
} // namespace mjs
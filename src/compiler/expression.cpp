#include "expression.h"

#include "lexer.h"
#include "expression_impl/binary_expression.h"
#include "expression_impl/yield_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<Expression> Expression::ParseExpression(Lexer* lexer) {
	return BinaryExpression::ParseExpressionAtCommaLevel(lexer);
}

std::optional<std::vector<std::string>> Expression::TryParseParameters(Lexer* lexer) {
	lexer->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer->PeekToken().is(TokenType::kSepRParen)) {
		do {
			// 处理剩余参数 (...args)
			if (lexer->PeekToken().is(TokenType::kSepEllipsis)) {
				lexer->NextToken(); // 消耗 ...
				if (lexer->PeekToken().is(TokenType::kIdentifier)) {
					parList.push_back("..." + lexer->MatchToken(TokenType::kIdentifier).value());
				} else {
					return std::nullopt;
				}
			}
			// 处理解构参数 ({ x, y } 或 [a, b])
			else if (lexer->PeekToken().is(TokenType::kSepLCurly) || lexer->PeekToken().is(TokenType::kSepLBrack)) {
				// 跳过解构模式，使用占位符
				lexer->NextToken(); // 消耗 { 或 [
				int depth = 1;
				while (depth > 0 && !lexer->PeekToken().is(TokenType::kEof)) {
					auto token = lexer->NextToken();
					if (token.is(TokenType::kSepLCurly) || token.is(TokenType::kSepLBrack)) {
						depth++;
					} else if (token.is(TokenType::kSepRCurly) || token.is(TokenType::kSepRBrack)) {
						depth--;
					}
				}
				parList.push_back("<destructured>");
			}
			// 处理普通标识符
			else if (lexer->PeekToken().is(TokenType::kIdentifier)) {
				parList.push_back(lexer->MatchToken(TokenType::kIdentifier).value());
			} else {
				return std::nullopt;
			}

			// 类型注解暂时跳过
			if (lexer->PeekToken().is(TokenType::kSepColon)) {
				lexer->NextToken();
				if (lexer->PeekToken().is(TokenType::kIdentifier)) {
					lexer->NextToken();
				}
			}

			// 跳过默认参数值 (= expression)
			if (lexer->PeekToken().is(TokenType::kOpAssign)) {
				lexer->NextToken(); // 消耗 =
				// 跳过默认值表达式（简单处理，跳到下一个逗号或右括号）
				int paren_depth = 0;
				int brace_depth = 0;
				int bracket_depth = 0;

				while (!lexer->PeekToken().is(TokenType::kEof)) {
					auto token = lexer->PeekToken();

					// 如果遇到逗号且在顶层，则停止
					if (token.is(TokenType::kSepComma) && paren_depth == 0 && brace_depth == 0 && bracket_depth == 0) {
						break;
					}

					// 如果遇到右括号且在顶层，则停止
					if (token.is(TokenType::kSepRParen) && paren_depth == 0 && brace_depth == 0 && bracket_depth == 0) {
						break;
					}

					// 更新深度
					if (token.is(TokenType::kSepLParen)) paren_depth++;
					else if (token.is(TokenType::kSepRParen)) paren_depth--;
					else if (token.is(TokenType::kSepLCurly)) brace_depth++;
					else if (token.is(TokenType::kSepRCurly)) brace_depth--;
					else if (token.is(TokenType::kSepLBrack)) bracket_depth++;
					else if (token.is(TokenType::kSepRBrack)) bracket_depth--;

					lexer->NextToken();
				}
			}

			if (!lexer->PeekToken().is(TokenType::kSepComma)) {
				break;
			}
			lexer->NextToken();
		} while (true);
	}

	auto token = lexer->PeekToken();
	if (!token.is(TokenType::kSepRParen)) {
		return std::nullopt;
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
			par_list.emplace_back(YieldExpression::ParseExpressionAtYieldLevel(lexer));
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
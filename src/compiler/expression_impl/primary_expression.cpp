#include "primary_expression.h"

#include <mjs/error.h>

#include "../lexer.h"
#include "../parser.h"
#include "identifier.h"
#include "undefined_literal.h"
#include "null_literal.h"
#include "boolean_literal.h"
#include "integer_literal.h"
#include "float_literal.h"
#include "string_literal.h"
#include "this_expression.h"
#include "super_expression.h"
#include "array_expression.h"
#include "object_expression.h"
#include "template_literal.h"
#include "class_expression.h"
#include "function_expression.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析基本表达式
 *
 * 基本表达式包括：
 * - 字面量：undefined, null, true, false, 数字, 字符串
 * - 标识符：变量名, 函数名等
 * - this表达式
 * - 括号表达式：(expression)
 * - 数组字面量：[item1, item2, ...]
 * - 对象字面量：{key1: value1, key2: value2, ...}
 * - 模板字符串：`template ${expression} string`
 *
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> PrimaryExpression::ParsePrimaryExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto token = lexer->PeekToken();

	switch (token.type()) {
	case TokenType::kKwClass: {
		// 类表达式
		return ClassExpression::ParseClassExpression(lexer, false);
	}
	case TokenType::kKwFunction:
	case TokenType::kKwAsync: {
		// 函数表达式
		return FunctionExpression::ParseExpressionAtFunctionLevel(lexer);
	}
	case TokenType::kUndefined: {
		lexer->NextToken();
		return std::make_unique<UndefinedLiteral>(start, lexer->GetRawSourcePosition());
	}
	case TokenType::kNull: {
		lexer->NextToken();
		return std::make_unique<NullLiteral>(start, lexer->GetRawSourcePosition());
	}
	case TokenType::kTrue: {
		lexer->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer->GetRawSourcePosition(), true);
	}
	case TokenType::kFalse: {
		lexer->NextToken();
		return std::make_unique<BooleanLiteral>(start, lexer->GetRawSourcePosition(), false);
	}
	case TokenType::kInteger: {
		lexer->NextToken();
		int64_t value = std::stoll(token.value(), nullptr, 0);
		return std::make_unique<IntegerLiteral>(start, lexer->GetRawSourcePosition(), value);
	}
	case TokenType::kFloat: {
		lexer->NextToken();
		double value = std::stod(token.value());
		return std::make_unique<FloatLiteral>(start, lexer->GetRawSourcePosition(), value);
	}
	case TokenType::kString: {
		lexer->NextToken();
		return std::make_unique<StringLiteral>(start, lexer->GetRawSourcePosition(),
										  std::string(token.value()));
	}
	case TokenType::kIdentifier: {
		return Identifier::ParseIdentifier(lexer);
	}
	case TokenType::kKwThis: {
		lexer->NextToken();
		return std::make_unique<ThisExpression>(start, lexer->GetRawSourcePosition());
	}
	case TokenType::kKwSuper: {
		lexer->NextToken();
		return std::make_unique<SuperExpression>(start, lexer->GetRawSourcePosition());
	}
	case TokenType::kSepLParen: {
		// 括号表达式
		lexer->NextToken(); // 消耗左括号
		auto exp = ParseExpression(lexer);
		lexer->MatchToken(TokenType::kSepRParen);
		return exp;
	}
	case TokenType::kSepLBrack: {
		// 数组字面量
		return ArrayExpression::ParseArrayExpression(lexer);
	}
	case TokenType::kSepLCurly: {
		// 对象字面量
		return ObjectExpression::ParseObjectExpression(lexer);
	}
	case TokenType::kBacktick: {
		// 模板字符串
		return TemplateLiteral::ParseTemplateLiteral(lexer);
	}
	default: {
		throw SyntaxError("Unexpected token: '{}'", Token::TypeToString(token.type()));
	}
	}
}

} // namespace compiler
} // namespace mjs
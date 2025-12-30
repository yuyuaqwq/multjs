#include "../expression.h"

#include "new_expression.h"
#include "import_expression.h"
#include "call_expression.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析new表达式、import表达式或成员表达式
 *
 * 这个函数处理以下几种表达式：
 * - new表达式：new Constructor(...)
 * - import表达式：import(...)
 * - 成员表达式：object.property, object[property]
 *
 * @param lexer 词法分析器
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> Expression::ParseExpressionAtLeftHandSideLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();

	// 处理new表达式
	if (lexer->PeekToken().is(TokenType::kKwNew)) {
		return NewExpression::ParseNewExpression(lexer);
	}

	// 处理import表达式
	if (lexer->PeekToken().is(TokenType::kKwImport)) {
		return ImportExpression::ParseImportExpression(lexer);
	}

	return CallExpression::ParseExpressionAtCallLevel(lexer, nullptr, true);
}

} // namespace compiler
} // namespace mjs
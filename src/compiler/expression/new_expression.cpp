#include "new_expression.h"

#include "../code_generator.h"
#include "call_expression.h"

namespace mjs {
namespace compiler {

void NewExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // new 表达式代码生成
    auto& new_exp = const_cast<NewExpression&>(*this);

    code_generator->GenerateParamList(function_def_base, new_exp.arguments());
    new_exp.callee()->GenerateCode(code_generator, function_def_base);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kNew);
}

/**
 * @brief 解析new表达式
 *
 * new表达式的形式为：new Constructor(args)
 *
 * @param lexer 词法分析器
 * @return 解析后的new表达式
 */
std::unique_ptr<Expression> NewExpression::ParseNewExpression(Lexer* lexer) {
	// new new ... 右结合
	auto start = lexer->GetSourcePosition();
	lexer->NextToken();
	std::unique_ptr<Expression> callee;

	auto type = lexer->PeekToken().type();
	if (type == TokenType::kKwNew) {
		callee = ParseNewExpression(lexer);
	}
	else {
		callee = CallExpression::ParseExpressionAtCallLevel(lexer, nullptr, false);
	}

	std::vector<std::unique_ptr<Expression>> arguments;
	if (lexer->PeekToken().is(TokenType::kSepLParen)) {
		arguments = Expression::ParseExpressions(lexer, TokenType::kSepLParen, TokenType::kSepRParen, false);
	}

	auto end = lexer->GetRawSourcePosition();
	auto exp = std::make_unique<NewExpression>(start, end, std::move(callee), std::move(arguments));

	// 后面可能还会跟函数调用之类的
	return CallExpression::ParseExpressionAtCallLevel(lexer, std::move(exp), true);
}

} // namespace compiler
} // namespace mjs
#include "yield_expression.h"

#include "../code_generator.h"
#include "function_expression.h"
#include "assignment_expression.h"

namespace mjs {
namespace compiler {

void YieldExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // yield 表达式代码生成
    auto& yield_exp = const_cast<YieldExpression&>(*this);
    yield_exp.argument()->GenerateCode(code_generator, function_def_base);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kYield);
}

std::unique_ptr<Expression> YieldExpression::ParseExpressionAtYieldLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto type = lexer->PeekToken().type();

	// 处理yield表达式
	if (type == TokenType::kKwYield) {
		return ParseYieldExpression(lexer);
	}

	// 处理函数表达式
	switch (type) {
	case TokenType::kIdentifier:
	case TokenType::kSepLParen:
	case TokenType::kKwAsync:
	case TokenType::kKwFunction:
		return FunctionExpression::ParseExpressionAtFunctionLevel(lexer);
	default:
		break;
	}

	// 处理赋值表达式
	return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
}

std::unique_ptr<YieldExpression> YieldExpression::ParseYieldExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->NextToken(); // 消耗yield关键字

	std::unique_ptr<Expression> yielded_value = AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<YieldExpression>(start, end, std::move(yielded_value));
}

} // namespace compiler
} // namespace mjs
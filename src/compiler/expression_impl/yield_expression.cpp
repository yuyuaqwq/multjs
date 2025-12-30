#include "yield_expression.h"

#include "../code_generator.h"
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
	// 处理yield表达式
	if (lexer->PeekToken().is(TokenType::kKwYield)) {
		return ParseYieldExpression(lexer);
	}

	// 否则继续解析赋值表达式
	return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
}

std::unique_ptr<YieldExpression> YieldExpression::ParseYieldExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->NextToken(); // 消耗yield关键字

	// 检查是否是 yield* (委托 yield)
	bool is_delegate = false;
	if (lexer->PeekToken().is(TokenType::kOpMul)) {
		lexer->NextToken(); // 消耗 *
		is_delegate = true;
	}

	std::unique_ptr<Expression> yielded_value = AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<YieldExpression>(start, end, std::move(yielded_value), is_delegate);
}

} // namespace compiler
} // namespace mjs
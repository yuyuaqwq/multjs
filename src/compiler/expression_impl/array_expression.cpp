#include "src/compiler/expression_impl/array_expression.h"

#include <mjs/object_impl/array_object.h>

#include "src/compiler/parser.h"
#include "src/compiler/code_generator.h"
#include "src/compiler/expression_impl/yield_expression.h"
#include "src/compiler/expression_impl/unary_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<ArrayExpression> ArrayExpression::ParseArrayExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kSepLBrack);

	std::vector<std::unique_ptr<Expression>> elements;

	// 解析数组元素
	while (!lexer->PeekToken().is(TokenType::kSepRBrack)) {
		if (lexer->PeekToken().is(TokenType::kSepComma)) {
			// 处理稀疏数组的空洞
			lexer->NextToken(); // 消耗逗号
			elements.push_back(nullptr); // 空洞用nullptr表示
			continue;
		}

		if (lexer->PeekToken().is(TokenType::kSepEllipsis)) {
			// 处理展开运算符 ...arr
			lexer->NextToken(); // 消耗...
			auto arg = YieldExpression::ParseExpressionAtYieldLevel(lexer);
			auto spread_end = lexer->GetRawSourcePosition();
			auto spread = std::make_unique<UnaryExpression>(
				arg->start(), spread_end, TokenType::kSepEllipsis, std::move(arg), true
			);
			elements.push_back(std::move(spread));
		} else {
			elements.push_back(YieldExpression::ParseExpressionAtYieldLevel(lexer));
		}

		// 检查是否有逗号分隔符
		if (lexer->PeekToken().is(TokenType::kSepComma)) {
			lexer->NextToken(); // 消耗逗号
		} else {
			break;
		}
	}

	lexer->MatchToken(TokenType::kSepRBrack);
	auto end = lexer->GetRawSourcePosition();

	return std::make_unique<ArrayExpression>(start, end, std::move(elements));
}

void ArrayExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 数组表达式代码生成

	code_generator->GenerateParamList(function_def_base, elements());

	auto literal_new = code_generator->AllocateConst(Value(ArrayObjectClassDef::LiteralNew));
	function_def_base->bytecode_table().EmitConstLoad(literal_new);
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
}

} // namespace compiler
} // namespace mjs
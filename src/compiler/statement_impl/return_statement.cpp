#include "return_statement.h"

#include "../code_generator.h"
#include "../expression_impl/yield_expression.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析return语句
 *
 * return语句的形式为：return [expression];
 *
 * @return 解析后的return语句
 */
std::unique_ptr<ReturnStatement> ReturnStatement::ParseReturnStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwReturn);
	std::unique_ptr<Expression> exp;
	if (!lexer->PeekToken().is(TokenType::kSepSemi)) {
		exp = YieldExpression::ParseExpressionAtYieldLevel(lexer);
	}
	lexer->MatchToken(TokenType::kSepSemi);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ReturnStatement>(start, end, std::move(exp));
}

void ReturnStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 生成返回值
    if (argument_) {
        code_generator->GenerateExpression(function_def_base, argument_.get());
    } else {
        // 无返回值，返回 undefined
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    }

    if (code_generator->IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kFunction })) {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFinallyReturn);
    }
    else {
        function_def_base->bytecode_table().EmitReturn(function_def_base);
    }
}

} // namespace compiler
} // namespace mjs
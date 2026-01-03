#include "src/compiler/statement_impl/if_statement.h"

#include "src/compiler/code_generator.h"
#include "src/compiler/expression_impl/yield_expression.h"

namespace mjs {
namespace compiler {

void IfStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 表达式结果压栈
    code_generator->GenerateExpression(function_def_base, test_.get());

    // 条件为false时，跳转到if块之后的地址
    auto if_pc = function_def_base->bytecode_table().Size();
    code_generator->GenerateIfEq(function_def_base);

    consequent_->GenerateCode(code_generator, function_def_base);

    if (alternate_) {
        // 跳过当前余下所有else if / else的指令
        auto end_pc = function_def_base->bytecode_table().Size();
         
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        function_def_base->bytecode_table().EmitPcOffset(0);

        // 修复条件为false时，跳转到if块之后的地址
        function_def_base->bytecode_table().RepairPc(if_pc, function_def_base->bytecode_table().Size());

        alternate_->GenerateCode(code_generator, function_def_base);

        function_def_base->bytecode_table().RepairPc(end_pc, function_def_base->bytecode_table().Size());
    }
    else {
        // 修复条件为false时，跳转到if块之后的地址
        function_def_base->bytecode_table().RepairPc(if_pc, function_def_base->bytecode_table().Size());
    }
}

std::unique_ptr<IfStatement> IfStatement::ParseIfStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->NextToken();

	lexer->MatchToken(TokenType::kSepLParen);
	auto test = YieldExpression::ParseExpressionAtYieldLevel(lexer);
	lexer->MatchToken(TokenType::kSepRParen);

	auto consequent = BlockStatement::ParseBlockStatement(lexer);

	std::unique_ptr<Statement> alternate;
	auto token = lexer->PeekToken();
	if (token.is(TokenType::kKwElse)) {
		lexer->NextToken();
		token = lexer->PeekToken();

		if (token.is(TokenType::kKwIf)) {
			alternate = IfStatement::ParseIfStatement(lexer);
		}
		else {
			alternate = BlockStatement::ParseBlockStatement(lexer);
		}
	}
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<IfStatement>(start, end, std::move(test), std::move(consequent), std::move(alternate));
}

} // namespace compiler
} // namespace mjs
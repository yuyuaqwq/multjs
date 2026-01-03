#include "src/compiler/expression_impl/conditional_expression.h"

#include "src/compiler/code_generator.h"
#include "src/compiler/statement.h"
#include "src/compiler/expression_impl/binary_expression.h"

namespace mjs {
namespace compiler {

void ConditionalExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 条件表达式代码生成
    auto& cond_exp = const_cast<ConditionalExpression&>(*this);

    // 生成条件测试表达式
    cond_exp.test()->GenerateCode(code_generator, function_def_base);

    // 条件为false时，跳转到else分支
    auto if_pc = function_def_base->bytecode_table().Size();
    code_generator->GenerateIfEq(function_def_base);

    // 生成条件为真时的表达式
    cond_exp.consequent()->GenerateCode(code_generator, function_def_base);

    // 跳过else分支
    auto skip_else_pc = function_def_base->bytecode_table().Size();
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    function_def_base->bytecode_table().EmitPcOffset(0);

    // 修复条件跳转，跳转到else分支
    function_def_base->bytecode_table().RepairPc(if_pc, function_def_base->bytecode_table().Size());

    // 生成条件为假时的表达式
    cond_exp.alternate()->GenerateCode(code_generator, function_def_base);

    // 修复跳过else分支的跳转
    function_def_base->bytecode_table().RepairPc(skip_else_pc, function_def_base->bytecode_table().Size());
}

std::unique_ptr<Expression> ConditionalExpression::ParseExpressionAtConditionalLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	// 三元，右结合
	auto test = BinaryExpression::ParseExpressionAtLogicalOrLevel(lexer);
	auto type = lexer->PeekToken().type();
	if (type != TokenType::kSepQuestion) {
		return test;
	}
	lexer->NextToken();
	auto consequent = ParseExpressionAtConditionalLevel(lexer);
	lexer->MatchToken(TokenType::kSepColon);
	auto alternate = ParseExpressionAtConditionalLevel(lexer);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ConditionalExpression>(start, end,
		std::move(test), std::move(consequent), std::move(alternate));
}

} // namespace compiler
} // namespace mjs
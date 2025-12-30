#include  "arrow_function_expression.h"

#include "../statement.h"
#include "../code_generator.h"
#include "../statement_impl/block_statement.h"
#include "../statement_impl/expression_statement.h"

#include "assignment_expression.h"
#include "yield_expression.h"

namespace mjs {
namespace compiler {

ArrowFunctionExpression::ArrowFunctionExpression(SourcePosition start, SourcePosition end,
	std::vector<std::string>&& params,
	std::unique_ptr<Statement> body,
	bool is_async)
	: Expression(start, end), params_(std::move(params)),
	body_(std::move(body)), is_async_(is_async) {
}

std::unique_ptr<Expression> ArrowFunctionExpression::TryParseArrowFunctionExpression(Lexer* lexer, SourcePosition start, bool is_async) {
	// 保存当前状态以便回退
	auto checkpoint = lexer->CreateCheckpoint();

	std::vector<std::string> params;

	// 解析参数
	if (lexer->PeekToken().is(TokenType::kSepLParen)) {
		auto res = Expression::TryParseParameters(lexer);
		if (!res) {
			// 不是箭头函数，回退
			lexer->RewindToCheckpoint(checkpoint);
			return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
		}
		params = *res;
	}
	else if (lexer->PeekToken().is(TokenType::kIdentifier)) {
		// 单个参数
		params.push_back(lexer->NextToken().value());
	}
	else {
		// 不是箭头函数，回退
		lexer->RewindToCheckpoint(checkpoint);
		return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	}

	// 检查是否有箭头 =>
	if (!lexer->PeekToken().is(TokenType::kSepArrow)) {
		// 不是箭头函数，回退
		lexer->RewindToCheckpoint(checkpoint);
		return AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	}

	// 确认是箭头函数
	lexer->NextToken(); // 跳过 =>

	// 解析函数体
	std::unique_ptr<Statement> body;
	if (lexer->PeekToken().is(TokenType::kSepLCurly)) {
		body = BlockStatement::ParseBlockStatement(lexer);
	} else {
		auto exp_start = lexer->GetSourcePosition();
		// 避免解析kSepComma
		auto exp = YieldExpression::ParseExpressionAtYieldLevel(lexer);
		auto exp_end = lexer->GetRawSourcePosition();
		body = std::make_unique<ExpressionStatement>(exp_start, exp_end, std::move(exp));
	}

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ArrowFunctionExpression>(
		start, end, std::move(params), std::move(body), is_async
	);
}

void ArrowFunctionExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 箭头函数表达式代码生成

	auto new_func_def = FunctionDef::New(&function_def_base->module_def(), "<anonymous_function>", params().size());
	auto const_idx = code_generator->AllocateConst(Value(new_func_def));

	new_func_def->set_is_arrow();
	if (is_async()) {
		new_func_def->set_is_async();
	}

	auto load_pc = function_def_base->bytecode_table().Size();
	function_def_base->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
	function_def_base->bytecode_table().EmitU32(const_idx);

	// 切换环境
	auto& scope = code_generator->EnterScope(function_def_base, new_func_def, ScopeType::kArrowFunction);

	// 参数正序分配
	for (auto& param : params()) {
		scope.AllocVar(param, VarFlags::kNone);
	}

	code_generator->GenerateFunctionBody(new_func_def, body().get());

	bool need_repair = new_func_def->has_this() || !new_func_def->closure_var_table().closure_var_defs().empty();

	// 恢复环境
	code_generator->ExitScope();
	new_func_def->debug_table().Sort();

	if (need_repair) {
		function_def_base->bytecode_table().RepairOpcode(load_pc, OpcodeType::kClosure);
	}
}

} // namespace compiler
} // namespace mjs
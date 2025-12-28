#include "for_statement.h"

#include "../code_generator.h"
#include "variable_declaration.h"
#include "expression_statement.h"
#include "../expression/assignment_expression.h"

namespace mjs {
namespace compiler {

void ForStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto save_loop_repair_entrys = code_generator->jump_manager().current_loop_repair_entries();

    std::vector<RepairEntry> loop_repair_entrys;
    code_generator->jump_manager().set_current_loop_repair_entries(&loop_repair_entrys);

    code_generator->EnterScope(function_def_base, nullptr, ScopeType::kFor);

    // init
    code_generator->GenerateStatement(function_def_base, init_.get());

    auto start_pc = function_def_base->bytecode_table().Size();

    // 表达式结果压栈
    if (test_) {
        code_generator->GenerateExpression(function_def_base, test_.get());
    }

    // 等待修复
    loop_repair_entrys.emplace_back(RepairEntry{
        .type = RepairEntry::Type::kBreak,
        .repair_pc = function_def_base->bytecode_table().Size(),
        });
    // 提前写入跳转的指令
    code_generator->GenerateIfEq(function_def_base);

    bool need_set_label = code_generator->jump_manager().current_label_reloop_pc() && code_generator->jump_manager().current_label_reloop_pc() == kInvalidPc;
    code_generator->jump_manager().set_current_label_reloop_pc(std::nullopt);

    body_->GenerateCode(code_generator, function_def_base);

    // 记录重新循环的pc
    auto reloop_pc = function_def_base->bytecode_table().Size();
    if (need_set_label) {
        code_generator->jump_manager().set_current_label_reloop_pc(reloop_pc);
    }

    if (update_) {
        code_generator->GenerateExpression(function_def_base, update_.get());
    }

    code_generator->ExitScope();

    // 重新回去看是否需要循环
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    function_def_base->bytecode_table().EmitPcOffset(0);
    function_def_base->bytecode_table().RepairPc(function_def_base->bytecode_table().Size() - 3, start_pc);

    code_generator->RepairEntries(function_def_base, loop_repair_entrys, function_def_base->bytecode_table().Size(), reloop_pc);

    code_generator->jump_manager().set_current_loop_repair_entries(save_loop_repair_entrys);
}

std::unique_ptr<ForStatement> ForStatement::ParseForStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwFor);
	lexer->MatchToken(TokenType::kSepLParen);

	std::unique_ptr<Statement> initialization;
	auto token = lexer->PeekToken();
	if (token.is(TokenType::kSepSemi)) {
		lexer->NextToken();
	}
	else {
		if (token.is(TokenType::kKwLet) || token.is(TokenType::kKwConst)) {
			initialization = VariableDeclaration::ParseVariableDeclaration(lexer, token.type());
		}
		else {
			initialization = ExpressionStatement::ParseExpressionStatement(lexer);
		}
	}

	token = lexer->PeekToken();
	std::unique_ptr<Expression> condition;
	if (!token.is(TokenType::kSepSemi)) {
		condition = AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	}
	lexer->MatchToken(TokenType::kSepSemi);

	token = lexer->PeekToken();
	std::unique_ptr<Expression> final_expression;
	if (!token.is(TokenType::kSepRParen)) {
		final_expression = AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	}
	lexer->MatchToken(TokenType::kSepRParen);

	auto block = BlockStatement::ParseBlockStatement(lexer);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ForStatement>(start, end,
		std::move(initialization), std::move(condition),
		std::move(final_expression), std::move(block));
}

} // namespace compiler
} // namespace mjs
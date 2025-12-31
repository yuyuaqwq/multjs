#include "while_statement.h"

#include "../code_generator.h"
#include "../expression_impl/yield_expression.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析while语句
 *
 * while语句的形式为：while (condition) { body }
 *
 * @return 解析后的while语句
 */
std::unique_ptr<WhileStatement> WhileStatement::ParseWhileStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwWhile);
	lexer->MatchToken(TokenType::kSepLParen);
	auto exp = YieldExpression::ParseExpressionAtYieldLevel(lexer);
	lexer->MatchToken(TokenType::kSepRParen);
	auto block = BlockStatement::ParseBlockStatement(lexer);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<WhileStatement>(start, end, std::move(exp), std::move(block));
}

void WhileStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto& jump_manager = code_generator->jump_manager();
    auto& scope_manager = code_generator->scope_manager();
    auto save_loop_repair_entrys = jump_manager.current_loop_repair_entries();

    std::vector<RepairEntry> loop_repair_entrys;
    jump_manager.set_current_loop_repair_entries(&loop_repair_entrys);

    // 记录重新循环的pc
    auto reloop_pc = function_def_base->bytecode_table().Size();
    if (jump_manager.current_label_reloop_pc() && jump_manager.current_label_reloop_pc() == kInvalidPc) {
        jump_manager.set_current_label_reloop_pc(reloop_pc);
    }

    // 表达式结果压栈
    code_generator->GenerateExpression(function_def_base, test_.get());

    // 等待修复
    loop_repair_entrys.emplace_back(RepairEntry{
        .type = RepairEntry::Type::kBreak,
        .repair_pc = function_def_base->bytecode_table().Size(),
        });
    // 提前写入跳转的指令
    code_generator->GenerateIfEq(function_def_base);

    scope_manager.EnterScope(function_def_base, nullptr, ScopeType::kWhile);
    body_->GenerateCode(code_generator, function_def_base);
    scope_manager.ExitScope();

    // 重新回去看是否需要循环
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    function_def_base->bytecode_table().EmitPcOffset(0);
    function_def_base->bytecode_table().RepairPc(function_def_base->bytecode_table().Size() - 3, reloop_pc);

    jump_manager.RepairEntries(function_def_base, loop_repair_entrys, function_def_base->bytecode_table().Size(), reloop_pc);

    jump_manager.set_current_loop_repair_entries(save_loop_repair_entrys);
}

} // namespace compiler
} // namespace mjs
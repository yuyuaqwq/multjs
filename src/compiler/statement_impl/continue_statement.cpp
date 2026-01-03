#include "src/compiler/statement_impl/continue_statement.h"

#include <mjs/error.h>

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析continue语句
 *
 * continue语句的形式为：continue [label];
 *
 * @return 解析后的continue语句
 */
std::unique_ptr<ContinueStatement> ContinueStatement::ParseContinueStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwContinue);
	std::optional<std::string> label_name;
	if (lexer->PeekToken().is(TokenType::kIdentifier)) {
		label_name = lexer->NextToken().value();
	}
	lexer->MatchToken(TokenType::kSepSemi);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ContinueStatement>(start, end, std::move(label_name));
}

void ContinueStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto& jump_manager = code_generator->jump_manager();
    if (jump_manager.current_loop_repair_entries() == nullptr) {
        throw SyntaxError("Cannot use break in acyclic scope");
    }

    if (label_) {
        auto iter = jump_manager.label_map().find(*label_);
        if (iter == jump_manager.label_map().end()) {
            throw SyntaxError("Label does not exist.");
        }
        iter->second.entries.emplace_back(RepairEntry{
            .type = RepairEntry::Type::kContinue,
            .repair_pc = function_def_base->bytecode_table().Size(),
            });
    }
    else {
        jump_manager.current_loop_repair_entries()->emplace_back(RepairEntry{
            .type = RepairEntry::Type::kContinue,
            .repair_pc = function_def_base->bytecode_table().Size(),
            });
    }

    // 跳到当前循环的末尾pc，等待修复
    if (code_generator->scope_manager().IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction, ScopeType::kArrowFunction })) {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFinallyGoto);
    }
    else {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    }
    function_def_base->bytecode_table().EmitPcOffset(0);
}

} // namespace compiler
} // namespace mjs
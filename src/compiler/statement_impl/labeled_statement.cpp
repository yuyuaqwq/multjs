#include "labeled_statement.h"

#include <mjs/error.h>

#include "../code_generator.h"

namespace mjs {
namespace compiler {

void LabeledStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto& jump_manager = code_generator->jump_manager();
    auto res = jump_manager.label_map().emplace(label_, LabelInfo());
    if (!res.second) {
        throw SyntaxError("Duplicate label.");
    }

    auto save_label_reloop_pc_ = jump_manager.current_label_reloop_pc();
    jump_manager.current_label_reloop_pc() = kInvalidPc;

    code_generator->GenerateStatement(function_def_base, body_.get());

    jump_manager.RepairEntries(function_def_base, res.first->second.entries, function_def_base->bytecode_table().Size(), *jump_manager.current_label_reloop_pc());

    jump_manager.label_map().erase(res.first);
    jump_manager.set_current_label_reloop_pc(save_label_reloop_pc_);
}

std::unique_ptr<LabeledStatement> LabeledStatement::ParseLabeledStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto label_name = lexer->MatchToken(TokenType::kIdentifier).value();
	lexer->MatchToken(TokenType::kSepColon);
	auto stat = Statement::ParseStatement(lexer);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<LabeledStatement>(start, end, std::move(label_name), std::move(stat));
}

} // namespace compiler
} // namespace mjs
#include "src/compiler/statement_impl/block_statement.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析块语句
 *
 * 块语句的形式为：{ statement1; statement2; ... }
 *
 * @return 解析后的块语句
 */
std::unique_ptr<BlockStatement> BlockStatement::ParseBlockStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Statement>> stat_list;

	while (!lexer->PeekToken().is(TokenType::kSepRCurly)) {
		stat_list.push_back(Statement::ParseStatement(lexer));
	}

	lexer->MatchToken(TokenType::kSepRCurly);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<BlockStatement>(start, end, std::move(stat_list));
}

void BlockStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    for (auto& stat : statements()) {
        stat->GenerateCode(code_generator, function_def_base);
    }
}

} // namespace compiler
} // namespace mjs
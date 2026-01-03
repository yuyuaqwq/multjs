#include "src/compiler/statement_impl/catch_clause.h"

#include "src/compiler/code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析catch子句
 *
 * catch子句的形式为：catch (error) { block }
 *
 * @return 解析后的catch子句
 */
std::unique_ptr<CatchClause> CatchClause::ParseCatchClause(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwCatch);

	lexer->MatchToken(TokenType::kSepLParen);
	auto exp = Identifier::ParseIdentifier(lexer);
	lexer->MatchToken(TokenType::kSepRParen);

	auto block = BlockStatement::ParseBlockStatement(lexer);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<CatchClause>(start, end, std::move(exp), std::move(block));
}

void CatchClause::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // CatchClause的代码生成逻辑已经在TryStatement中处理
    // 这里不需要实现，因为TryStatement已经处理了catch块的代码生成
}

} // namespace compiler
} // namespace mjs
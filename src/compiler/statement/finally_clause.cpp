#include "finally_clause.h"

#include "../code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析finally子句
 *
 * finally子句的形式为：finally { block }
 *
 * @return 解析后的finally子句
 */
std::unique_ptr<FinallyClause> FinallyClause::ParseFinallyClause(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwFinally);
	auto block = BlockStatement::ParseBlockStatement(lexer);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<FinallyClause>(start, end, std::move(block));
}

void FinallyClause::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // FinallyClause的代码生成逻辑已经在TryStatement中处理
    // 这里不需要实现，因为TryStatement已经处理了finally块的代码生成
}

} // namespace compiler
} // namespace mjs
#include "expression_statement.h"


#include "../code_generator.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析表达式语句
 *
 * 表达式语句的形式为：expression;
 *
 * @return 解析后的表达式语句
 */
std::unique_ptr<ExpressionStatement> ExpressionStatement::ParseExpressionStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	if (lexer->PeekToken().is(TokenType::kSepSemi)) {
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		return std::make_unique<ExpressionStatement>(start, end, nullptr);
	}
	auto exp = Expression::ParseExpression(lexer);
	lexer->MatchToken(TokenType::kSepSemi);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ExpressionStatement>(start, end, std::move(exp));
}

void ExpressionStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto exp = expression_.get();
    if (!exp) {
        // 空语句
        return;
    }
    // 抛弃纯表达式语句的最终结果
    code_generator->GenerateExpression(function_def_base, exp);
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

} // namespace compiler
} // namespace mjs
#include "throw_statement.h"

#include "../code_generator.h"
#include "../expression/assignment_expression.h"

namespace mjs {
namespace compiler {

/**
 * @brief 解析throw语句
 *
 * throw语句的形式为：throw expression;
 *
 * @return 解析后的throw语句
 */
std::unique_ptr<ThrowStatement> ThrowStatement::ParseThrowStatement(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	lexer->MatchToken(TokenType::kKwThrow);
	auto exp = AssignmentExpression::ParseExpressionAtAssignmentLevel(lexer);
	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<ThrowStatement>(start, end, std::move(exp));
}

void ThrowStatement::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    code_generator->GenerateExpression(function_def_base, argument_.get());
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kThrow);
}

} // namespace compiler
} // namespace mjs
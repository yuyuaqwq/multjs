#include "assignment_expression.h"

#include "../lexer.h"
#include "../code_generator.h"
#include "../statement.h"
#include "conditional_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<Expression> AssignmentExpression::ParseExpressionAtAssignmentLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	// 赋值，右结合
	auto exp = ConditionalExpression::ParseExpressionAtConditionalLevel(lexer);
	auto op = lexer->PeekToken().type();
	if (op != TokenType::kOpAssign) {
		return exp;
	}
	lexer->NextToken();
	auto end = lexer->GetRawSourcePosition();
	exp = std::make_unique<AssignmentExpression>(start, end, op, std::move(exp), ParseExpressionAtAssignmentLevel(lexer));
	return exp;
}

void AssignmentExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 赋值表达式代码生成
    auto& assign_exp = const_cast<AssignmentExpression&>(*this);

    // 右值表达式先入栈
    assign_exp.right()->GenerateCode(code_generator, function_def_base);

    auto lvalue_exp = assign_exp.left().get();
    code_generator->GenerateLValueStore(function_def_base, lvalue_exp);
}

} // namespace compiler
} // namespace mjs
#include "assignment_expression.h"

#include "../lexer.h"
#include "../code_generator.h"
#include "../statement.h"
#include "conditional_expression.h"
#include "arrow_function_expression.h"

namespace mjs {
namespace compiler {

std::unique_ptr<Expression> AssignmentExpression::ParseExpressionAtAssignmentLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();

	// 检查是否是箭头函数 (参数列表或单个参数)
	if (lexer->PeekToken().is(TokenType::kSepLParen) ||
		lexer->PeekToken().is(TokenType::kIdentifier)) {
		// 尝试解析为箭头函数
		auto arrow_func = ArrowFunctionExpression::TryParseArrowFunctionExpression(lexer, start, false);
		if (arrow_func != nullptr) {
			// 成功解析了箭头函数
			return arrow_func;
		}
		// 不是箭头函数，继续作为普通表达式解析
	}

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
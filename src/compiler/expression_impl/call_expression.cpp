#include "call_expression.h"

#include "../code_generator.h"
#include "../statement.h"
#include "member_expression.h"
#include "primary_expression.h"
#include "super_expression.h"

namespace mjs {
namespace compiler {

void CallExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 函数调用表达式代码生成
    auto& call_exp = const_cast<CallExpression&>(*this);

    // 生成参数列表
    code_generator->GenerateParamList(function_def_base, call_exp.arguments());
    call_exp.callee()->GenerateCode(code_generator, function_def_base);

    // 将this置于栈顶
    if (dynamic_cast<MemberExpression*>(call_exp.callee().get())) {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
    }
    else if (dynamic_cast<SuperExpression*>(call_exp.callee().get())) {
        // super() 调用时，this已经在栈上（由SuperExpression::GenerateCode生成kGetSuper）
        // 不需要压入undefined
    }
    else {
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    }

    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
}

/**
 * @brief 解析成员表达式或调用表达式
 *
 * 这个函数处理以下几种表达式：
 * - 成员访问：object.property
 * - 计算属性：object[property]
 * - 函数调用：function(args)
 * - 可选链：object?.property, object?.[property], function?.()
 *
 * @param lexer 词法分析器
 * @param right 右侧表达式
 * @param match_lparen 是否需要匹配左括号（用于嵌套调用）
 * @return 解析后的表达式
 */
std::unique_ptr<Expression> CallExpression::ParseExpressionAtCallLevel(
	Lexer* lexer, std::unique_ptr<Expression> right, bool match_lparen) {
	if (!right) {
		right = PrimaryExpression::ParsePrimaryExpression(lexer);
	}

	do {
		auto token = lexer->PeekToken();
		if (token.is(TokenType::kSepDot) || token.is(TokenType::kSepLBrack)) {
			right = MemberExpression::ParseMemberExpression(lexer, std::move(right));
		}
		else if (match_lparen && token.is(TokenType::kSepLParen)) {
			right = ParseCallExpression(lexer, std::move(right));
		}
		else {
			break;
		}
	} while (true);
	return right;
}

/**
 * @brief 解析函数调用表达式
 *
 * 函数调用表达式的形式为：callee(arg1, arg2, ...)
 *
 * @param lexer 词法分析器
 * @param callee 被调用的函数表达式
 * @return 解析后的调用表达式
 */
std::unique_ptr<CallExpression> CallExpression::ParseCallExpression(Lexer* lexer, std::unique_ptr<Expression> callee) {
	auto start = lexer->GetSourcePosition();

	auto arguments = Expression::ParseExpressions(lexer, TokenType::kSepLParen, TokenType::kSepRParen, false);

	auto end = lexer->GetRawSourcePosition();
	return std::make_unique<CallExpression>(start, end, std::move(callee), std::move(arguments));
}

} // namespace compiler
} // namespace mjs
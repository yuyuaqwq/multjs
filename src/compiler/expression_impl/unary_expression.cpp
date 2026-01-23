#include "src/compiler/expression_impl/unary_expression.h"

#include "src/compiler/code_generator.h"
#include "src/compiler/expression_impl/await_expression.h"

namespace mjs {
namespace compiler {

void UnaryExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 一元表达式代码生成
    auto& unary_exp = const_cast<UnaryExpression&>(*this);

    // 表达式的值入栈
    unary_exp.argument()->GenerateCode(code_generator, function_def_base);

    // 生成运算指令
    switch (unary_exp.op()) {
    case TokenType::kOpSub:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kNeg);
        break;
    case TokenType::kKwTypeof:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kTypeof);
        break;
    case TokenType::kOpPrefixInc:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kInc);
        code_generator->GenerateLValueStore(function_def_base, unary_exp.argument().get());
        break;
    case TokenType::kOpSuffixInc:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kDump);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kInc);
        code_generator->GenerateLValueStore(function_def_base, unary_exp.argument().get());
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPop);
        break;
    default:
        throw SyntaxError("Unsupported unary operator");
    }
}


std::unique_ptr<Expression> UnaryExpression::ParseExpressionAtUnaryLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto token = lexer->PeekToken();

	// 处理前缀一元运算符
	switch (token.type()) {
	case TokenType::kKwAwait: {    // await
		lexer->NextToken();
		auto argument = ParseExpressionAtUnaryLevel(lexer);
		auto end = lexer->GetRawSourcePosition();
		return std::make_unique<AwaitExpression>(start, end, std::move(argument));
	}
	case TokenType::kOpAdd:      // +
	case TokenType::kOpSub:      // -
	case TokenType::kOpNot:      // !
	case TokenType::kOpBitNot:   // ~
	case TokenType::kKwTypeof:   // typeof
	case TokenType::kKwVoid:     // void
	case TokenType::kKwDelete: {  // delete
		lexer->NextToken();
		auto argument = ParseExpressionAtUnaryLevel(lexer);
		auto end = lexer->GetRawSourcePosition();
		return std::make_unique<UnaryExpression>(start, end, token.type(), std::move(argument), true);
	}
	case TokenType::kOpInc:      // ++
	case TokenType::kOpDec: {     // --
		lexer->NextToken();
		auto argument = ParseExpressionAtUnaryLevel(lexer);

		auto end = lexer->GetRawSourcePosition();

		// 使用特殊的前缀自增/自减标记
		TokenType prefix_op = (token.type() == TokenType::kOpInc) ?
			TokenType::kOpPrefixInc : TokenType::kOpPrefixDec;

		return std::make_unique<UnaryExpression>(
			start, end, prefix_op, std::move(argument), true
		);
	}
	default:
		// 如果不是一元运算符，则解析后缀表达式
		return ParsePostfixExpression(lexer);
	}
}

std::unique_ptr<Expression> UnaryExpression::ParsePostfixExpression(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = Expression::ParseExpressionAtLeftHandSideLevel(lexer);
	do {
		auto type = lexer->PeekToken().type();
		if (type != TokenType::kOpInc
			&& type != TokenType::kOpDec) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<UnaryExpression>(start, end, TokenType::kOpSuffixInc, std::move(exp), false);
	} while (true);
	return exp;
}


} // namespace compiler
} // namespace mjs
#include "binary_expression.h"

#include "../statement.h"
#include "yield_expression.h"
#include "assignment_expression.h"
#include "unary_expression.h"

namespace mjs {
namespace compiler {

void BinaryExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // 二元表达式代码生成
    auto& binary_exp = const_cast<BinaryExpression&>(*this);

    // 左右表达式的值入栈
    binary_exp.left()->GenerateCode(code_generator, function_def_base);
    binary_exp.right()->GenerateCode(code_generator, function_def_base);

    // 生成运算指令
    switch (binary_exp.op()) {
    case TokenType::kOpAdd:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kAdd);
        break;
    case TokenType::kOpSub:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSub);
        break;
    case TokenType::kOpMul:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kMul);
        break;
    case TokenType::kOpDiv:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kDiv);
        break;
    case TokenType::kOpEq:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kEq);
        break;
    case TokenType::kOpNe:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kNe);
        break;
    case TokenType::kOpLt:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kLt);
        break;
    case TokenType::kOpGt:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGt);
        break;
    case TokenType::kOpLe:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kLe);
        break;
    case TokenType::kOpGe:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kGe);
        break;
    case TokenType::kSepComma:
        break;
    case TokenType::kOpShiftLeft:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kShl);
        break;
    case TokenType::kOpShiftRight:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kShr);
        break;
    case TokenType::kOpUnsignedShiftRight:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUShr);
        break;
    case TokenType::kOpBitAnd:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kBitAnd);
        break;
    case TokenType::kOpBitOr:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kBitOr);
        break;
    case TokenType::kOpBitXor:
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kBitXor);
        break;
    default:
        throw std::runtime_error("Unsupported binary operator");
    }
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtCommaLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = YieldExpression::ParseExpressionAtYieldLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kSepComma) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), YieldExpression::ParseExpressionAtYieldLevel(lexer));

	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtLogicalOrLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtLogicalAndLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpOr
			&& op != TokenType::kOpNullishCoalescing) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtLogicalAndLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtLogicalAndLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtBitwiseOrLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpAnd) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtBitwiseOrLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtBitwiseOrLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtBitwiseXorLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpBitOr) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtBitwiseXorLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtBitwiseXorLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtBitwiseAndLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpBitXor) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtBitwiseAndLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtBitwiseAndLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtEqualityLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpBitAnd) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtEqualityLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtEqualityLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtRelationalLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpNe
			&& op != TokenType::kOpEq
			&& op != TokenType::kOpStrictEq
			&& op != TokenType::kOpStrictNe) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtRelationalLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtRelationalLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtShiftLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpLt
			&& op != TokenType::kOpLe
			&& op != TokenType::kOpGt
			&& op != TokenType::kOpGe
			&& op != TokenType::kKwIn
			&& op != TokenType::kKwInstanceof) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtShiftLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtShiftLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtAdditiveLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpShiftLeft
			&& op != TokenType::kOpShiftRight
			&& op != TokenType::kOpUnsignedShiftRight) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtAdditiveLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtAdditiveLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtMultiplicativeLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpAdd
			&& op != TokenType::kOpSub) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtMultiplicativeLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtMultiplicativeLevel(Lexer* lexer) {
	auto start = lexer->GetSourcePosition();
	auto exp = ParseExpressionAtExponentiationLevel(lexer);
	do {
		auto op = lexer->PeekToken().type();
		if (op != TokenType::kOpMul
			&& op != TokenType::kOpDiv
			&& op != TokenType::kOpMod) {
			break;
		}
		lexer->NextToken();
		auto end = lexer->GetRawSourcePosition();
		exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtExponentiationLevel(lexer));
	} while (true);
	return exp;
}

std::unique_ptr<Expression> BinaryExpression::ParseExpressionAtExponentiationLevel(Lexer* lexer) {
	// .. ** ..，右结合
	auto start = lexer->GetSourcePosition();
	auto exp = UnaryExpression::ParseExpressionAtUnaryLevel(lexer);
	auto op = lexer->PeekToken().type();
	if (op != TokenType::kOpPower) {
		return exp;
	}
	lexer->NextToken();
	auto end = lexer->GetRawSourcePosition();
	exp = std::make_unique<BinaryExpression>(start, end, op, std::move(exp), ParseExpressionAtExponentiationLevel(lexer));
	return exp;
}

} // namespace compiler
} // namespace mjs
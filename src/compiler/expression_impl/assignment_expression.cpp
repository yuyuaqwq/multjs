#include "src/compiler/expression_impl/assignment_expression.h"

#include <mjs/opcode.h>

#include "src/compiler/lexer.h"
#include "src/compiler/code_generator.h"
#include "src/compiler/statement.h"
#include "src/compiler/expression_impl/conditional_expression.h"
#include "src/compiler/expression_impl/arrow_function_expression.h"

namespace mjs {
namespace compiler {

namespace {
	// 检查是否是赋值运算符
	bool IsAssignmentOperator(TokenType op) {
		return op == TokenType::kOpAssign ||
			   op == TokenType::kOpAddAssign ||
			   op == TokenType::kOpSubAssign ||
			   op == TokenType::kOpMulAssign ||
			   op == TokenType::kOpDivAssign ||
			   op == TokenType::kOpModAssign ||
			   op == TokenType::kOpPowerAssign ||
			   op == TokenType::kOpBitAndAssign ||
			   op == TokenType::kOpBitOrAssign ||
			   op == TokenType::kOpBitXorAssign ||
			   op == TokenType::kOpShiftLeftAssign ||
			   op == TokenType::kOpShiftRightAssign ||
			   op == TokenType::kOpUnsignedShiftRightAssign;
	}
}

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
	// 检查是否是赋值运算符（包括复合赋值运算符）
	if (!IsAssignmentOperator(op)) {
		return exp;
	}
	lexer->NextToken();
	auto end = lexer->GetRawSourcePosition();
	exp = std::make_unique<AssignmentExpression>(start, end, op, std::move(exp), ParseExpressionAtAssignmentLevel(lexer));
	return exp;
}

void AssignmentExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    auto& assign_exp = const_cast<AssignmentExpression&>(*this);
    auto lvalue_exp = assign_exp.left().get();

    // 如果是复合赋值运算符，需要先加载左值，执行运算，然后存储
    if (operator_ != TokenType::kOpAssign) {
        // 1. 加载左值表达式的值
        code_generator->GenerateExpression(function_def_base, lvalue_exp);

        // 2. 加载右值表达式的值
        assign_exp.right()->GenerateCode(code_generator, function_def_base);

        // 3. 执行相应的二元运算
        OpcodeType opcode;
        switch (operator_) {
            case TokenType::kOpAddAssign:
                opcode = OpcodeType::kAdd;
                break;
            case TokenType::kOpSubAssign:
                opcode = OpcodeType::kSub;
                break;
            case TokenType::kOpMulAssign:
                opcode = OpcodeType::kMul;
                break;
            case TokenType::kOpDivAssign:
                opcode = OpcodeType::kDiv;
                break;
            case TokenType::kOpModAssign:
                // TODO: 支持 Mod 运算
                throw SyntaxError("Modulo assignment not yet implemented");
            case TokenType::kOpPowerAssign:
                // TODO: 支持 Power 运算
                throw SyntaxError("Power assignment not yet implemented");
            case TokenType::kOpBitAndAssign:
                opcode = OpcodeType::kBitAnd;
                break;
            case TokenType::kOpBitOrAssign:
                opcode = OpcodeType::kBitOr;
                break;
            case TokenType::kOpBitXorAssign:
                opcode = OpcodeType::kBitXor;
                break;
            case TokenType::kOpShiftLeftAssign:
                opcode = OpcodeType::kShl;
                break;
            case TokenType::kOpShiftRightAssign:
                opcode = OpcodeType::kShr;
                break;
            case TokenType::kOpUnsignedShiftRightAssign:
                opcode = OpcodeType::kUShr;
                break;
            default:
                throw SyntaxError("Unsupported assignment operator");
        }
        function_def_base->bytecode_table().EmitOpcode(opcode);

        // 4. 将运算结果存储回左值
        code_generator->GenerateLValueStore(function_def_base, lvalue_exp);
    } else {
        // 普通赋值运算符：直接计算右值，然后存储到左值
        assign_exp.right()->GenerateCode(code_generator, function_def_base);
        code_generator->GenerateLValueStore(function_def_base, lvalue_exp);
    }
}

} // namespace compiler
} // namespace mjs
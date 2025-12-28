#include <mjs/token.h>

#include "./parser.h"

namespace mjs {
namespace compiler {

std::string Token::TypeToString(TokenType type) {
	// 首先在运算符映射表中查找
	for (const auto& [op_str, op_type] : Token::operator_map()) {
		if (op_type == type) {
			return op_str;
		}
	}
	
	// 然后在关键字映射表中查找
	for (const auto& [keyword_str, keyword_type] : Token::keyword_map()) {
		if (keyword_type == type) {
			return keyword_str;
		}
	}
	
	// 特殊处理
	switch (type) {
		case TokenType::kNone:
			return "[none]";
		case TokenType::kEof:
			return "[eof]";
		case TokenType::kInteger:
			return "[integer]";
		case TokenType::kFloat:
			return "[float]";
		case TokenType::kBigInt:
			return "[bigint]";
		case TokenType::kString:
			return "[string]";
		case TokenType::kRegExp:
			return "[regexp]";
		case TokenType::kIdentifier:
			return "[identifier]";
		case TokenType::kTemplateElement:
			return "[template_element]";
		default:
			return "[unknown]";
	}
}

const std::unordered_map<std::string, TokenType>& Token::operator_map() {
	static std::unordered_map<std::string, TokenType> operator_map = {
		{ ";", TokenType::kSepSemi },
		{ ":", TokenType::kSepColon },
		{ ",", TokenType::kSepComma },
		{ ".", TokenType::kSepDot },
		{ "(", TokenType::kSepLParen },
		{ ")", TokenType::kSepRParen },
		{ "[", TokenType::kSepLBrack },
		{ "]", TokenType::kSepRBrack },
		{ "{", TokenType::kSepLCurly },
		{ "}", TokenType::kSepRCurly },
		{ "+", TokenType::kOpAdd },
		{ "++", TokenType::kOpInc },
		{ "-", TokenType::kOpSub },
		{ "--", TokenType::kOpDec },
		{ "*", TokenType::kOpMul },
		{ "**", TokenType::kOpPower },
		{ "/", TokenType::kOpDiv },
		{ "%", TokenType::kOpMod },
		{ "!", TokenType::kOpNot },
		{ "=", TokenType::kOpAssign },
		{ "==", TokenType::kOpEq },
		{ "===", TokenType::kOpStrictEq },
		{ "!=", TokenType::kOpNe },
		{ "!==", TokenType::kOpStrictNe },
		{ "<", TokenType::kOpLt },
		{ "<=", TokenType::kOpLe },
		{ ">", TokenType::kOpGt },
		{ ">=", TokenType::kOpGe },
		{ "~", TokenType::kOpBitNot },
		{ "&", TokenType::kOpBitAnd },
		{ "|", TokenType::kOpBitOr },
		{ "^", TokenType::kOpBitXor },
		{ "<<", TokenType::kOpShiftLeft },
		{ ">>", TokenType::kOpShiftRight },
		{ ">>>", TokenType::kOpUnsignedShiftRight },
		{ "&&", TokenType::kOpAnd },
		{ "||", TokenType::kOpOr },
		{ "??", TokenType::kOpNullishCoalescing },
		{ "?.", TokenType::kOpOptionalChain },
		{ "?", TokenType::kSepQuestion },
		{ "=>", TokenType::kSepArrow },
		{ "...", TokenType::kSepEllipsis },
		// 复合赋值运算符
		{ "+=", TokenType::kOpAddAssign },
		{ "-=", TokenType::kOpSubAssign },
		{ "*=", TokenType::kOpMulAssign },
		{ "/=", TokenType::kOpDivAssign },
		{ "%=", TokenType::kOpModAssign },
		{ "**=", TokenType::kOpPowerAssign },
		{ "&=", TokenType::kOpBitAndAssign },
		{ "|=", TokenType::kOpBitOrAssign },
		{ "^=", TokenType::kOpBitXorAssign },
		{ "<<=", TokenType::kOpShiftLeftAssign },
		{ ">>=", TokenType::kOpShiftRightAssign },
		{ ">>>=", TokenType::kOpUnsignedShiftRightAssign },
	};
	return operator_map;
}

const std::unordered_map<std::string, TokenType>& Token::keyword_map() {
	static std::unordered_map<std::string, TokenType> keyword_map = {
		{ "undefined", TokenType::kUndefined },
		{ "true", TokenType::kTrue },
		{ "false", TokenType::kFalse },
		{ "null", TokenType::kNull },
		{ "if", TokenType::kKwIf },
		{ "else", TokenType::kKwElse },
		{ "function", TokenType::kKwFunction },
		{ "for", TokenType::kKwFor },
		{ "while", TokenType::kKwWhile },
		{ "continue", TokenType::kKwContinue },
		{ "break", TokenType::kKwBreak },
		{ "return", TokenType::kKwReturn },
		{ "try", TokenType::kKwTry },
		{ "catch", TokenType::kKwCatch },
		{ "finally", TokenType::kKwFinally },
		{ "throw", TokenType::kKwThrow },
		{ "let", TokenType::kKwLet },
		{ "const", TokenType::kKwConst },
		{ "yield", TokenType::kKwYield },
		{ "async", TokenType::kKwAsync },
		{ "await", TokenType::kKwAwait },
		{ "this", TokenType::kKwThis },
		{ "new", TokenType::kKwNew },
		{ "class", TokenType::kKwClass },
		{ "delete", TokenType::kKwDelete },
		{ "typeof", TokenType::kKwTypeof },
		{ "instanceof", TokenType::kKwInstanceof },
		{ "in", TokenType::kKwIn },
		{ "void", TokenType::kKwVoid },
		{ "with", TokenType::kKwWith },
		{ "switch", TokenType::kKwSwitch },
		{ "case", TokenType::kKwCase },
		{ "default", TokenType::kKwDefault },
		{ "import", TokenType::kKwImport },
		{ "as", TokenType::kKwAs },
		{ "from", TokenType::kKwFrom },
		{ "export", TokenType::kKwExport },
	};
	return keyword_map;
}

//std::unique_ptr<Statement> Token::Parse(Parser& parser) {
//    // 使用token工厂创建具体的token子类，然后调用其Parse方法
//    auto specific_token = TokenFactory::CreateTokenFromToken(*this);
//    return specific_token->Parse(parser);
//}

} // namespace compiler
} // namespace mjs
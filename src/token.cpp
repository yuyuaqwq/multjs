#include "token.h"

namespace mjs {

bool Token::Is(TokenType type) const noexcept {
	return type_ == type;
}


std::unordered_map<std::string, TokenType> g_operators = {
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
};

std::unordered_map<std::string, TokenType> g_keywords = {
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
	// { "var", TokenType::kKwVar },
	{ "let", TokenType::kKwLet },
	{ "const", TokenType::kKwConst },
	{ "yield", TokenType::kKwYield, },
	{ "async", TokenType::kKwAsync },
	{ "await", TokenType::kKwAwait },
	{ "this", TokenType::kKwThis, },
	{ "new", TokenType::kKwNew, },

	
	{ "default", TokenType::kKwDefault },

	{ "import", TokenType::kKwImport },
	{ "as", TokenType::kKwAs },
	{ "from", TokenType::kKwFrom },
	{ "export", TokenType::kKwExport },
};

} // namespace msj
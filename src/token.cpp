#include "token.h"

namespace mjs {

bool Token::Is(TokenType type) const noexcept {
	return type_ == type;
}

std::map<std::string, TokenType> g_keywords = {
	{ "if", TokenType::kKwIf },
	{ "else", TokenType::kKwElse },
	{ "function", TokenType::kKwFunction },
	{ "for", TokenType::kKwFor },
	{ "while", TokenType::kKwWhile },
	{ "continue", TokenType::kKwContinue },
	{ "break", TokenType::kKwBreak },
	{ "return", TokenType::kKwReturn },
	// { "var", TokenType::kKwVar },
	{ "let", TokenType::kKwLet },
	{ "yield", TokenType::kKwYield, },
	{ "async", TokenType::kKwAsync },
	{ "await", TokenType::kKwAwait },
	{ "this", TokenType::kKwThis, },
	{ "new", TokenType::kKwNew, },
};

} // namespace msj
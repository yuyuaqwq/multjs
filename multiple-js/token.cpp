#include "token.h"

namespace mjs {

bool Token::Is(TokenType t_type) const noexcept {
	return t_type == type;
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
	{ "var", TokenType::kKwVar },
	{ "let", TokenType::kKwLet },
};

} // namespace msj
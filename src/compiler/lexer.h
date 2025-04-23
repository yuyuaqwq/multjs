#pragma once

#include <string>
#include <stdexcept>

#include <mjs/noncopyable.h>

#include "token.h"

namespace mjs {
namespace compiler {

using SourcePos = uint32_t;
using SourceLine = uint32_t;

class LexerException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Lexer : public noncopyable {
public:
	Lexer(const char* src);
	~Lexer() noexcept;

	Token PeekToken();
	Token PeekTokenN(uint32_t n);
	Token NextToken();
	Token MatchToken(TokenType type);

	SourcePos pos() const { return pos_; }

private:
	char NextChar() noexcept;
	char PeekChar() noexcept;
	void SkipChar(int count) noexcept;
	bool TestStr(const std::string& str);
	bool TestChar(char c);

	Token ReadNextToken();

private:
	std::string src_;
	SourcePos pos_ = 0;
	Token peek_;
	SourceLine line_ = 1;
};

} // namespace compiler
} // namespace mjs
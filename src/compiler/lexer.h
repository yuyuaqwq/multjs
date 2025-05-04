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
	struct Checkpoint {
		SourcePos pos;
		SourceLine line;
		Token cur_token;
		Token peek_;
	};

public:
	Lexer(const char* src);
	~Lexer() noexcept;

	Token PeekToken();
	Token PeekTokenN(uint32_t n);
	Token NextToken();
	Token MatchToken(TokenType type);

	Checkpoint CreateCheckpoint() {
		return Checkpoint{
			.pos = pos_,
			.line = line_,
			.cur_token = cur_token_,
			.peek_ = peek_,
		};
	}

	void RewindToCheckpoint(const Checkpoint& checkpoint) {
		pos_ = checkpoint.pos;
		line_ = checkpoint.line;
		cur_token_ = checkpoint.cur_token;
		peek_ = checkpoint.peek_;
	}

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
	SourceLine line_ = 1;
	Token cur_token_;
	Token peek_;
};

} // namespace compiler
} // namespace mjs
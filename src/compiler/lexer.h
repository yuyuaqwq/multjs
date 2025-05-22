#pragma once

#include <string>
#include <stdexcept>

#include <mjs/noncopyable.h>
#include <mjs/source_def.h>

#include "token.h"

namespace mjs {
namespace compiler {

class Lexer : public noncopyable {
public:
	struct Checkpoint {
		SourcePos pos;
		SourcePos peek_pos;
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
			.peek_pos = peek_pos_,
			.line = line_,
			.cur_token = cur_token_,
			.peek_ = peek_,
		};
	}

	void RewindToCheckpoint(const Checkpoint& checkpoint) {
		pos_ = checkpoint.pos;
		peek_pos_ = checkpoint.peek_pos;
		line_ = checkpoint.line;
		cur_token_ = checkpoint.cur_token;
		peek_ = checkpoint.peek_;
	}

	//SourcePos GetSourcePosWithSkipUselessString() { 
	//	// 跳过无效字符
	//	SkipUselessStr();
	//	return pos_;
	//}

	SourcePos GetSourcePos() {
		// 跳过无效字符
		SkipUselessStr();
		return pos_;
	}

private:
	char NextChar() noexcept;
	char PeekChar() noexcept;
	void SkipChar(int count) noexcept;
	bool TestStr(const std::string& str);
	bool TestStr(const char* str, size_t size);
	bool TestChar(char c);

	void SkipUselessStr();

	Token ReadNextToken();

private:
	std::string src_;
	SourcePos pos_ = 0;
	SourcePos peek_pos_ = 0;
	SourceLine line_ = 1;
	Token cur_token_;
	Token peek_;
};

} // namespace compiler
} // namespace mjs
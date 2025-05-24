#pragma once

#include <string>
#include <stdexcept>

#include <mjs/noncopyable.h>
#include <mjs/source.h>

#include "token.h"

namespace mjs {
namespace compiler {

class Lexer : public noncopyable {
public:
	struct Checkpoint {
		SourcePos pos;
		SourcePos peek_pos;
		Token cur_token;
		Token peek;
		bool in_template;
		bool in_template_interpolation;
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
			.cur_token = cur_token_,
			.peek = peek_,
			.in_template = in_template_,
			.in_template_interpolation = in_template_interpolation_,
		};
	}

	void RewindToCheckpoint(const Checkpoint& checkpoint) {
		pos_ = checkpoint.pos;
		peek_pos_ = checkpoint.peek_pos;
		cur_token_ = checkpoint.cur_token;
		peek_ = checkpoint.peek;
		in_template_ = checkpoint.in_template;
		in_template_interpolation_ = checkpoint.in_template_interpolation;
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

	SourcePos GetRawSourcePos() {
		return pos_;
	}

private:
	char NextChar() noexcept;
	char PeekChar() noexcept;
	void SkipChar(int count) noexcept;
	bool TestStr(std::string_view string);
	bool TestChar(char c);

	void SkipUselessStr();

	Token ReadNextToken();

	std::string ReadString(char quote_type, std::initializer_list<std::string_view> end_strings = {});

private:
	std::string src_;
	SourcePos pos_ = 0;
	SourcePos peek_pos_ = 0;
	Token cur_token_;
	Token peek_;

	bool in_template_ = false;
	bool in_template_interpolation_ = false;
};

} // namespace compiler
} // namespace mjs
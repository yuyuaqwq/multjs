#pragma once

#include <string>
#include <map>

namespace mjs {

enum class TokenType {
	kNil = 0,

	kEof,
	kNull,
	kFalse,
	kTrue,
	kNumber,
	kString,
	kIdentifier,	// [a-zA-Z_][a-zA-Z0-9_]*

	kDecimalLiteral, // IntegerLiteral '.' [0-9]* | '.' [0-9]+ | IntegerLiteral
	kIntegerLiteral, // '0' | [1-9] [0-9]*

	kSepSemi,		// ;
	kSepComma,		// ,
	kSepDot,		// .
	kSepColon,		// :

	kSepLParen,     // (
	kSepRParen,     // )
	kSepLBrack,		// [
	kSepRBrack,		// ]
	kSepLCurly,		// {
	kSepRCurly,		// }

	kOpAssign,		// =
	kOpAdd,			// +
	kOpSub,			// -
	kOpMul,			// *
	kOpDiv,			// /

	kOpNe,			// !=
	kOpEq,			// ==
	kOpLt,			// <
	kOpLe,			// <=
	kOpGt,			// >
	kOpGe,			// >=

	kKwFunction,	// function
	kKwIf,			// if
	kKwElse,		// else
	kKwWhile,		// while
	kKwFor,			// for
	kKwContinue,	// continue
	kKwBreak,		// break
	kKwReturn,		// return
	kKwVar,			// var
	kKwLet,			// let
	kKwImport,		// import
	kKwClass,		// class
};

class Token {
public:
	bool Is(TokenType type) const noexcept;

	TokenType type() const { return type_; }
	void set_type(TokenType type) { type_ = type; }

	int32_t line() const { return line_; }
	void set_line(int32_t line) { line_ = line; }

	std::string* mutable_str() { return &str_; }
	const std::string& str() const { return str_; }
	void set_str(std::string str) { str_ = std::move(str); }

private:
	int32_t line_ = 0;		// 行号
	TokenType type_ = TokenType::kNil;		// token类型
	std::string str_;	// 保存必要的信息
};

extern std::map<std::string, TokenType> g_keywords;

} // namespace msj
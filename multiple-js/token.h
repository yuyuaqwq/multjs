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

	kOpNewVar,		// :=
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
};

struct Token {
	bool Is(TokenType type) const noexcept;

	int32_t line;		// 行号
	TokenType type;		// token类型
	std::string str;	// 保存必要的信息
};

extern std::map<std::string, TokenType> g_keywords;

} // namespace msj
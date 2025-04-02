#pragma once

#include <exception>
#include <memory>
#include <vector>

#include <mjs/noncopyable.h>

#include "lexer.h"
#include "stat.h"
#include "exp.h"

namespace mjs {

class ParserException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Parser : public noncopyable {
public:
	Parser(Lexer* t_lexer);

	std::unique_ptr<BlockStat> ParseSource();
	std::unique_ptr<BlockStat> ParseBlockStat();
	std::unique_ptr<Stat> ParseStat();

	std::unique_ptr<ExpStat> ParseExpStat();

	std::vector<std::string> ParseParNameList();
	std::unique_ptr<IfStat> ParseIfStat();
	std::unique_ptr<ElseIfStat> ParseElseIfStat();
	std::unique_ptr<ElseStat> ParseElseStat();

	std::unique_ptr<ForStat> ParseForStat();
	std::unique_ptr<WhileStat> ParseWhileStat();
	std::unique_ptr<ContinueStat> ParseContinueStat();
	std::unique_ptr<BreakStat> ParseBreakStat();
	std::unique_ptr<ReturnStat> ParseReturnStat();

	std::unique_ptr<NewVarStat> ParseNewVarStat();

	std::unique_ptr<Exp> ParseExp();
	std::unique_ptr<Exp> ParseExp20();
	std::unique_ptr<Exp> ParseExp19();
	std::unique_ptr<Exp> ParseExp18();
	std::unique_ptr<Exp> ParseExp17();
	std::unique_ptr<Exp> ParseExp16();
	std::unique_ptr<Exp> ParseExp15();
	std::unique_ptr<Exp> ParseExp14();
	std::unique_ptr<Exp> ParseExp13();
	std::unique_ptr<Exp> ParseExp12();
	std::unique_ptr<Exp> ParseExp11();
	std::unique_ptr<Exp> ParseExp10();
	std::unique_ptr<Exp> ParseExp9();
	std::unique_ptr<Exp> ParseExp8();
	std::unique_ptr<Exp> ParseExp7();
	std::unique_ptr<Exp> ParseExp6();
	std::unique_ptr<Exp> ParseExp5();
	std::unique_ptr<Exp> ParseExp4();

	
	std::unique_ptr<Exp> ParseNewExp();
	std::unique_ptr<Exp> ParseExp3();
	
	std::unique_ptr<Exp> ParseCallExp();
	std::unique_ptr<Exp> ParseMemberExp();
	std::unique_ptr<Exp> ParseExp2();

	std::unique_ptr<Exp> ParseExp1();

	std::unique_ptr<Exp> ParsePrimaryExp();

	std::unique_ptr<FuncDeclExp> ParseFunctionDeclExp();
	std::vector<std::unique_ptr<Exp>> ParseExpList(TokenType begin, TokenType end, bool allow_comma_end);

private:
	Lexer* lexer_;
};

} // namespace mjs
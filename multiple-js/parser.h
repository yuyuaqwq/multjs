#pragma once

#include <exception>
#include <memory>
#include <vector>

#include "lexer.h"
#include "stat.h"
#include "exp.h"

namespace mjs {

class ParserException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Parser {
public:
	Parser(Lexer* t_lexer);

	std::unique_ptr<BlockStat> ParseSource();
	std::unique_ptr<BlockStat> ParseBlockStat();
	std::unique_ptr<Stat> ParseStat();

	std::unique_ptr<ExpStat> ParseExpStat();

	std::unique_ptr<FuncDeclStat> ParseFunctionDeclStat();
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
	std::unique_ptr<Exp> ParseExp4();
	std::unique_ptr<Exp> ParseExp3();
	std::unique_ptr<Exp> ParseExp2();
	std::unique_ptr<Exp> ParseExp1();
	std::unique_ptr<Exp> ParseExp0();
	std::vector<std::unique_ptr<Exp>> ParseParExpList();

private:
	Lexer* lexer_;
};

} // namespace mjs
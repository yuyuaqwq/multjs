#pragma once

#include <exception>
#include <memory>
#include <vector>

#include <mjs/noncopyable.h>

#include "lexer.h"
#include "stat.h"
#include "exp.h"

namespace mjs {
namespace compiler {

class ParserException : public std::exception {
public:
	using Base = std::exception;
	using Base::Base;
};

class Parser : public noncopyable {
public:
	Parser(Lexer* t_lexer);

	void ParseSource();
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
	std::unique_ptr<TryStat> ParseTryStat();
	std::unique_ptr<CatchStat> ParseCatchStat();
	std::unique_ptr<FinallyStat> ParseFinallyStat();
	std::unique_ptr<ThrowStat> ParseThrowStat();

	std::unique_ptr<LabelStat> ParseLabelStat();
	std::unique_ptr<NewVarStat> ParseNewVarStat(TokenType type);

	std::unique_ptr<Stat> ParseImportStat(TokenType type);
	std::unique_ptr<ExportStat> ParseExportStat(TokenType type);

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
	
	std::unique_ptr<Exp> ParseExp2(bool match_lparen);

	std::unique_ptr<Exp> ParseExp1();

	std::unique_ptr<IdentifierExp> ParseIdentifierExp();
	std::unique_ptr<Exp> ParsePrimaryExp();

	std::unique_ptr<FunctionDeclExp> ParseFunctionDeclExp();
	std::vector<std::unique_ptr<Exp>> ParseExpList(TokenType begin, TokenType end, bool allow_comma_end);

	const auto& src_stats() const { return src_stats_; }
	const auto& import_stats() const { return import_stats_; }

private:
	Lexer* lexer_;

	std::vector<std::unique_ptr<Stat>> src_stats_;
	std::vector<std::unique_ptr<ImportStat>> import_stats_;
};

} // namespace compiler
} // namespace mjs
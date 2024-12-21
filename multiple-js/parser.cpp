#include "parser.h"

/* EBNF
exp = addexp
addexp = mulexp {oper2 mulexp}
oper2 = '+' | '-'
mulexp = parenexp {oper1 parenexp}
oper1 = '*' | '/'
parenexp = '(' addexp ')' | numexp
numexp = number
*/

namespace mjs {

Parser::Parser(Lexer* lexer)
	: lexer_(lexer) {}


std::unique_ptr<BlockStat> Parser::ParseSource() {
	std::vector<std::unique_ptr<Stat>> stat_list;

	while (!lexer_->LookAHead().Is(TokenType::kEof)) {
		stat_list.push_back(ParseStat());
	}

	return std::make_unique<BlockStat>(std::move(stat_list));
}

std::unique_ptr<BlockStat> Parser::ParseBlockStat() {
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Stat>> stat_list;

	while (!lexer_->LookAHead().Is(TokenType::kSepRCurly)) {
		stat_list.push_back(ParseStat());
	}

	lexer_->MatchToken(TokenType::kSepRCurly);

	return std::make_unique<BlockStat>(std::move(stat_list));
}

std::unique_ptr<Stat> Parser::ParseStat() {
	auto token = lexer_->LookAHead();

	switch (token.type) {
		case TokenType::kKwFunction: {
			return ParseFuncDefStat();
		}
		case TokenType::kSepLCurly: {
			return ParseBlockStat();
		}
		case TokenType::kKwIf: {
			return ParseIfStat();
		}
		case TokenType::kKwFor: {
			return ParseForStat();
		}
		case TokenType::kKwWhile: {
			return ParseWhileStat();
		}
		case TokenType::kKwContinue: {
			return ParseContinueStat();
		}
		case TokenType::kKwBreak: {
			return ParseBreakStat();
		}
		case TokenType::kKwReturn: {
			return ParseReturnStat();
		}
		case TokenType::kIdentifier: {
			auto temo_lexer = *lexer_;
			temo_lexer.NextToken();
			auto token = temo_lexer.LookAHead();
			if (token.Is(TokenType::kOpNewVar)){
				return ParseNewVarStat();
			}
			if (token.Is(TokenType::kOpAssign)) {
				return ParseAssignStat();
			}
			// break;
		}
		default: {
			return ParseExpStat();
		}
	}
}

std::unique_ptr<ExpStat> Parser::ParseExpStat() {
	if (lexer_->LookAHead().type == TokenType::kSepSemi) {
		lexer_->NextToken();
		return std::make_unique<ExpStat>(nullptr);
	}
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ExpStat>(move(exp));
}

std::unique_ptr<FuncDefStat> Parser::ParseFuncDefStat() {
	lexer_->MatchToken(TokenType::kKwFunction);
	auto func_name = lexer_->MatchToken(TokenType::kIdentifier).str;
	auto par_list = ParseParNameList();
	auto block = ParseBlockStat();
	return std::make_unique<FuncDefStat>(func_name, par_list, std::move(block));
}

std::vector<std::string> Parser::ParseParNameList() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->LookAHead().Is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).str);
			if (!lexer_->LookAHead().Is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return parList;
}

std::unique_ptr<IfStat> Parser::ParseIfStat() {
	lexer_->NextToken();

	auto exp = ParseExp();
	auto block = ParseBlockStat();

	std::vector<std::unique_ptr<ElseIfStat>> else_if_stat_list;

	auto token = lexer_->LookAHead();

	std::unique_ptr<ElseStat> elseStat;
	while (token.Is(TokenType::kKwElse)) {
		lexer_->NextToken();
		auto token = lexer_->LookAHead();
		if (token.Is(TokenType::kKwIf)) {
			else_if_stat_list.push_back(ParseElseIfStat());
			token = lexer_->LookAHead();
		}
		else {
			break;
		}
		elseStat = ParseElseStat();
	}

	return std::make_unique<IfStat>(std::move(exp), std::move(block), std::move(else_if_stat_list), std::move(elseStat));
}

std::unique_ptr<ElseIfStat> Parser::ParseElseIfStat() {
	lexer_->MatchToken(TokenType::kKwIf);
	auto exp = ParseExp();
	auto block = ParseBlockStat();
	return std::make_unique<ElseIfStat>(std::move(exp), std::move(block));
}

std::unique_ptr<ElseStat> Parser::ParseElseStat() {
	auto block = ParseBlockStat();
	return std::make_unique<ElseStat>(std::move(block));
}

std::unique_ptr<ForStat> Parser::ParseForStat() {
	lexer_->MatchToken(TokenType::kKwFor);
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str;
	lexer_->MatchToken(TokenType::kSepColon);
	auto exp = ParseExp();
	auto block = ParseBlockStat();
	return  std::make_unique<ForStat>(var_name, move(exp), move(block));
}

std::unique_ptr<WhileStat> Parser::ParseWhileStat() {
	lexer_->MatchToken(TokenType::kKwWhile);
	auto exp = ParseExp();
	auto block = ParseBlockStat();
	return std::make_unique<WhileStat>(move(exp), move(block));
}

std::unique_ptr<ContinueStat> Parser::ParseContinueStat() {
	lexer_->MatchToken(TokenType::kKwContinue);
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ContinueStat>();
}

std::unique_ptr<BreakStat> Parser::ParseBreakStat() {
	lexer_->MatchToken(TokenType::kKwBreak);
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<BreakStat>();
}

std::unique_ptr<ReturnStat> Parser::ParseReturnStat() {
	lexer_->MatchToken(TokenType::kKwReturn);
	std::unique_ptr<Exp> exp;
	if (lexer_->LookAHead().type != TokenType::kSepSemi) {
		exp = ParseExp();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ReturnStat>(move(exp));
}

std::unique_ptr<NewVarStat> Parser::ParseNewVarStat() {
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str;
	lexer_->MatchToken(TokenType::kOpNewVar);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return  std::make_unique<NewVarStat>(var_name, move(exp));
}

std::unique_ptr<AssignStat> Parser::ParseAssignStat() {
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str;
	lexer_->MatchToken(TokenType::kOpAssign);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return  std::make_unique<AssignStat>(var_name, move(exp));
}


std::unique_ptr<Exp> Parser::ParseExp() {
	return ParseExp3();
}

std::unique_ptr<Exp> Parser::ParseExp3() {
	auto exp = ParseExp2();
	do {
		auto type = lexer_->LookAHead().type;
		if (type != TokenType::kOpNe && type != TokenType::kOpEq && type != TokenType::kOpLt && type != TokenType::kOpLe && type != TokenType::kOpGt && type != TokenType::kOpGe) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaOpExp>(move(exp), type, ParseExp2());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp2() {
	auto exp = ParseExp1();
	
	do {
		auto type = lexer_->LookAHead().type;
		if (type != TokenType::kOpAdd && type != TokenType::kOpSub) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaOpExp>(move(exp), type, ParseExp1());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp1() {
	auto exp = ParseExp0();
	do {
		auto type = lexer_->LookAHead().type;
		if (type != TokenType::kOpMul && type != TokenType::kOpDiv) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaOpExp>(move(exp), type, ParseExp0());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp0() {
	auto token = lexer_->LookAHead();

	switch (token.type) {
	case TokenType::kNull: {
		lexer_->NextToken();
		return std::make_unique<NullExp>();
	}
	case TokenType::kTrue: {
		lexer_->NextToken();
		return std::make_unique<BoolExp>(true);
	}
	case TokenType::kFalse: {
		lexer_->NextToken();
		return std::make_unique<BoolExp>(false);
	}
	case TokenType::kNumber: {
		lexer_->NextToken();
		return std::make_unique<NumberExp>(atoi(token.str.c_str()));
	}
	case TokenType::kString: {
		lexer_->NextToken();
		return std::make_unique<StringExp>(token.str);
	}
	case TokenType::kIdentifier: {
		lexer_->NextToken();
		if (lexer_->LookAHead().Is(TokenType::kSepLParen)) {
			auto func_name = token.str;
			auto par_list = ParseParExpList();
			return std::make_unique<FunctionCallExp>(func_name, std::move(par_list));
		}
		return std::make_unique<NameExp>(token.str);
	}
	default: {
		throw ParserException("Unable to parse expression");
	}
	}
}

std::vector<std::unique_ptr<Exp>> Parser::ParseParExpList() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::unique_ptr<Exp>> par_list;
	if (!lexer_->LookAHead().Is(TokenType::kSepRParen)) {
		do {
			par_list.push_back(ParseExp());
			if (!lexer_->LookAHead().Is(TokenType::kSepComma)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(TokenType::kSepRParen);
	return par_list;
}

} // namespace parser

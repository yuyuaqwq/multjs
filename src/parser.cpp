#include "parser.h"

/* EBNF
exp = exp3
exp3 = exp2 {oper3 exp2}
oper3 = '==' | '!='
exp2 = exp1 {oper2 exp1}
oper2 = '+' | '-'
exp1 = exp0 {oper1 exp0}
oper1 = '*' | '/'
exp0 = '(' exp ')' | exp0
exp0 = number
*/

namespace mjs {

Parser::Parser(Lexer* lexer)
	: lexer_(lexer) {}


std::unique_ptr<BlockStat> Parser::ParseSource() {
	std::vector<std::unique_ptr<Stat>> stat_list;

	while (!lexer_->PeekToken().Is(TokenType::kEof)) {
		stat_list.push_back(ParseStat());
	}

	return std::make_unique<BlockStat>(std::move(stat_list));
}

std::unique_ptr<BlockStat> Parser::ParseBlockStat() {
	lexer_->MatchToken(TokenType::kSepLCurly);

	std::vector<std::unique_ptr<Stat>> stat_list;

	while (!lexer_->PeekToken().Is(TokenType::kSepRCurly)) {
		stat_list.push_back(ParseStat());
	}

	lexer_->MatchToken(TokenType::kSepRCurly);

	return std::make_unique<BlockStat>(std::move(stat_list));
}

std::unique_ptr<Stat> Parser::ParseStat() {
	auto token = lexer_->PeekToken();
	switch (token.type()) {
		case TokenType::kKwFunction: {
			return ParseFunctionDeclStat();
		}
		case TokenType::kKwLet: {
			return ParseNewVarStat();
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
			
			// break;
		}
		default: {
			return ParseExpStat();
		}
	}
}

std::unique_ptr<ExpStat> Parser::ParseExpStat() {
	if (lexer_->PeekToken().Is(TokenType::kSepSemi)) {
		lexer_->NextToken();
		return std::make_unique<ExpStat>(nullptr);
	}
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ExpStat>(move(exp));
}

std::unique_ptr<FuncDeclStat> Parser::ParseFunctionDeclStat() {
	lexer_->MatchToken(TokenType::kKwFunction);
	auto func_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	auto par_list = ParseParNameList();
	auto block = ParseBlockStat();
	return std::make_unique<FuncDeclStat>(func_name, par_list, std::move(block));
}

std::vector<std::string> Parser::ParseParNameList() {
	lexer_->MatchToken(TokenType::kSepLParen);
	std::vector<std::string> parList;
	if (!lexer_->PeekToken().Is(TokenType::kSepRParen)) {
		do {
			parList.push_back(lexer_->MatchToken(TokenType::kIdentifier).str());
			if (!lexer_->PeekToken().Is(TokenType::kSepComma)) {
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

	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStat();

	std::vector<std::unique_ptr<ElseIfStat>> else_if_stat_list;

	auto token = lexer_->PeekToken();

	std::unique_ptr<ElseStat> else_stat;
	while (token.Is(TokenType::kKwElse)) {
		lexer_->NextToken();
		token = lexer_->PeekToken();
		if (token.Is(TokenType::kKwIf)) {
			else_if_stat_list.push_back(ParseElseIfStat());
			token = lexer_->PeekToken();
			continue;
		}
		else_stat = ParseElseStat();
	}
	return std::make_unique<IfStat>(std::move(exp), std::move(block), std::move(else_if_stat_list), std::move(else_stat));
}

std::unique_ptr<ElseIfStat> Parser::ParseElseIfStat() {
	lexer_->MatchToken(TokenType::kKwIf);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStat();
	return std::make_unique<ElseIfStat>(std::move(exp), std::move(block));
}

std::unique_ptr<ElseStat> Parser::ParseElseStat() {
	auto block = ParseBlockStat();
	return std::make_unique<ElseStat>(std::move(block));
}

std::unique_ptr<ForStat> Parser::ParseForStat() {
	lexer_->MatchToken(TokenType::kKwFor);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kSepColon);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStat();
	return  std::make_unique<ForStat>(var_name, move(exp), move(block));
}

std::unique_ptr<WhileStat> Parser::ParseWhileStat() {
	lexer_->MatchToken(TokenType::kKwWhile);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepRParen);
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
	if (!lexer_->PeekToken().Is(TokenType::kSepSemi)) {
		exp = ParseExp();
	}
	lexer_->MatchToken(TokenType::kSepSemi);
	return std::make_unique<ReturnStat>(move(exp));
}

std::unique_ptr<NewVarStat> Parser::ParseNewVarStat() {
	lexer_->MatchToken(TokenType::kKwLet);
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kOpAssign);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return  std::make_unique<NewVarStat>(var_name, move(exp));
}


std::unique_ptr<Exp> Parser::ParseExp() {
	return ParseExp19();
}

std::unique_ptr<Exp> Parser::ParseExp19() {
	auto exp = ParseExp18();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kSepComma) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp18());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp18() {
	// 赋值，右结合
	auto exp = ParseExp17();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpAssign) {
		return exp;
	}
	lexer_->NextToken();
	exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp18());
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp17() {
	// 三元，右结合
	auto exp = ParseExp16();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kSepQuestion) {
		return exp;
	}
	lexer_->NextToken();
	auto exp2 = ParseExp17();
	lexer_->MatchToken(TokenType::kSepColon);
	auto exp3 = ParseExp17();
	exp = std::make_unique<TernaryOpExp>(TokenType::kOpTernary, move(exp), std::move(exp2), std::move(exp3));
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp16() {
	auto exp = ParseExp15();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpOr
			&& type != TokenType::kOpNullishCoalescing) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp15());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp15() {
	auto exp = ParseExp14();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpAnd) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp14());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp14() {
	auto exp = ParseExp13();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitOr) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp13());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp13() {
	auto exp = ParseExp12();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitXor) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp12());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp12() {
	auto exp = ParseExp11();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpBitAnd) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp11());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp11() {
	auto exp = ParseExp10();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpNe
			&& type != TokenType::kOpEq
			&& type != TokenType::kOpStrictEq
			&& type != TokenType::kOpStrictNe) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp10());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp10() {
	auto exp = ParseExp9();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpLt
			&& type != TokenType::kOpLe
			&& type != TokenType::kOpGt
			&& type != TokenType::kOpGe
			&& type != TokenType::kKwIn
			&& type != TokenType::kKwInstanceof) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp9());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp9() {
	auto exp = ParseExp8();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpShiftLeft
			&& type != TokenType::kOpShiftRight
			&& type != TokenType::kOpUnsignedShiftRight) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp8());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp8() {
	auto exp = ParseExp7();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpAdd
			&& type != TokenType::kOpSub) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp7());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp7() {
	auto exp = ParseExp6();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpMul
			&& type != TokenType::kOpDiv
			&& type != TokenType::kOpMod) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(move(exp), type, ParseExp6());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp6() {
	// .. ** ..，右结合
	auto exp = ParseExp5();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpPower) {
		return exp;
	}
	lexer_->NextToken();
	exp = std::make_unique<UnaryOpExp>(type, ParseExp6());
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp5() {
	auto token = lexer_->PeekToken();
	std::unique_ptr<Exp> exp;
	switch (token.type()) {
	case TokenType::kOpNot:
	case TokenType::kOpBitNot:
	case TokenType::kOpAdd:
	case TokenType::kOpSub:
	case TokenType::kKwTypeof:
	case TokenType::kKwVoid:
	case TokenType::kKwDelete:
	case TokenType::kKwAwait: {
		lexer_->NextToken();
		exp = std::make_unique<UnaryOpExp>(token.type(), ParseExp5());
		break;
	}
	case TokenType::kOpInc: {
		lexer_->NextToken();
		exp = ParseExp5();
		if (exp->value_category != ExpValueCategory::kLeftValue) {
			throw ParserException("Only use auto inc on lvalue.");
		}
		exp = std::make_unique<UnaryOpExp>(TokenType::kOpPrefixInc, std::move(exp));
		break;
	}
	case TokenType::kOpDec: {
		lexer_->NextToken();
		exp = ParseExp5();
		if (exp->value_category != ExpValueCategory::kLeftValue) {
			throw ParserException("Only use auto dec on lvalue.");
		}
		exp = std::make_unique<UnaryOpExp>(TokenType::kOpPrefixDec, std::move(exp));
		break;
	}
	default:
		exp = ParseExp4();
	}
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp4() {
	auto exp = ParseExp3();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kOpInc
			&& type != TokenType::kOpDec) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<UnaryOpExp>(TokenType::kOpSuffixInc, move(exp));
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp3() {
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kKwNew) {
		return ParseExp2();
	}
	lexer_->NextToken();
	auto exp = std::make_unique<UnaryOpExp>(type, ParseExp3());
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp2() {
	auto exp = ParseExp1();
	do {
		auto type = lexer_->PeekToken().type();
		if (type == TokenType::kSepDot) {
			lexer_->NextToken();
			auto exp2 = ParseExp0();
			if (exp2->GetType() != ExpType::kIdentifier) {
				throw ParserException("cannot match identifier.");
			}
			// 还可能是对标识符的函数调用
			if (lexer_->PeekToken().Is(TokenType::kSepLParen)) {
				auto par_list = ParseExpList(TokenType::kSepLParen, TokenType::kSepRParen, false);
				exp2 = std::make_unique<FunctionCallExp>(std::move(exp2), std::move(par_list));
			}
			auto exp2_type = exp2->GetType();
			exp = std::make_unique<DotExp>(move(exp), std::move(exp2));
			if (exp2_type == ExpType::kIdentifier) {
				exp->value_category = ExpValueCategory::kLeftValue;
			}
			else {
				// 如果是函数调用表达式，就是右值
				exp->value_category = ExpValueCategory::kRightValue;
			}
		}
		else if (type == TokenType::kSepLParen) {
			auto par_list = ParseExpList(TokenType::kSepLParen, TokenType::kSepRParen, false);
			exp = std::make_unique<FunctionCallExp>(std::move(exp), std::move(par_list));
		}
		else if (type == TokenType::kSepLBrack) {
			lexer_->NextToken();
			auto index_exp = ParseExp();
			lexer_->MatchToken(TokenType::kSepRBrack);
			exp = std::make_unique<IndexedExp>(std::move(exp), std::move(index_exp));
			exp->value_category = ExpValueCategory::kLeftValue;
		}
		else {
			break;
		}
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp1() {
	auto type = lexer_->PeekToken().type();
	if (lexer_->PeekToken().Is(TokenType::kSepLParen)) {
		lexer_->NextToken();
		auto exp = ParseExp();
		lexer_->MatchToken(TokenType::kSepRParen);
		return exp;
	}
	else {
		return ParseExp0();
	}
}

std::unique_ptr<Exp> Parser::ParseExp0() {
	auto token = lexer_->PeekToken();
	std::unique_ptr<Exp> exp;
	switch (token.type()) {
	case TokenType::kNull: {
		lexer_->NextToken();
		exp = std::make_unique<NullExp>();
		break;
	}
	case TokenType::kTrue: {
		lexer_->NextToken();
		exp = std::make_unique<BoolExp>(true);
		break;
	}
	case TokenType::kFalse: {
		lexer_->NextToken();
		exp = std::make_unique<BoolExp>(false);
		break;
	}
	case TokenType::kNumber: {
		lexer_->NextToken();
		exp = std::make_unique<NumberExp>(std::stod(token.str()));
		break;
	}
	case TokenType::kString: {
		lexer_->NextToken();
		exp = std::make_unique<StringExp>(token.str());
		break;
	}
	case TokenType::kSepLBrack: {
		// 数组字面量
		auto arr_literal = ParseExpList(TokenType::kSepLBrack, TokenType::kSepRBrack, true);
		exp = std::make_unique<ArrayLiteralExp>(std::move(arr_literal));
		break;
	}
	case TokenType::kSepLCurly: {
		// 可能是对象字面量
		lexer_->NextToken();
		std::unordered_map<std::string, std::unique_ptr<Exp>> obj_literal;
		if (!lexer_->PeekToken().Is(TokenType::kSepRCurly)) {
			do {
				auto ident = lexer_->MatchToken(TokenType::kIdentifier);
				lexer_->MatchToken(TokenType::kSepColon);
				// ParseExp19会解析kSepComma，避免
				obj_literal.emplace(ident.str(), ParseExp18());
				if (!lexer_->PeekToken().Is(TokenType::kSepComma)) {
					break;
				}
				if (lexer_->PeekToken().Is(TokenType::kSepRCurly)) {
					break;
				}
				lexer_->NextToken();
			} while (true);
		}
		lexer_->MatchToken(TokenType::kSepRCurly);
		exp = std::make_unique<ObjectLiteralExp>(std::move(obj_literal));
		break;
	}
	case TokenType::kIdentifier: {
		lexer_->NextToken();
		exp = std::make_unique<IdentifierExp>(token.str());
		exp->value_category = ExpValueCategory::kLeftValue;
		break;
	}
	}

	if (!exp) {
		throw ParserException("Unable to parse expression.");
	}
	return exp;
}

std::vector<std::unique_ptr<Exp>> Parser::ParseExpList(TokenType begin, TokenType end, bool allow_comma_end) {
	lexer_->MatchToken(begin);
	std::vector<std::unique_ptr<Exp>> par_list;
	if (!lexer_->PeekToken().Is(end)) {
		do {
			// ParseExp19会解析kSepComma，避免
			par_list.emplace_back(ParseExp18());
			if (!lexer_->PeekToken().Is(TokenType::kSepComma)) {
				break;
			}
			if (allow_comma_end && lexer_->PeekToken().Is(end)) {
				break;
			}
			lexer_->NextToken();
		} while (true);
	}
	lexer_->MatchToken(end);
	return par_list;
}

} // namespace parser
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
		case TokenType::kKwTry: {
			return ParseTryStat();
		}
		case TokenType::kKwThrow: {
			return ParseThrowStat();
		}
		case TokenType::kKwAsync:
		case TokenType::kKwFunction: {
			// 如果是直接定义，就不需要添加分号
			return std::make_unique<ExpStat>(ParseFunctionDeclExp());
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
	return std::make_unique<ExpStat>(std::move(exp));
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
		break;
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
	return  std::make_unique<ForStat>(var_name, std::move(exp), std::move(block));
}

std::unique_ptr<WhileStat> Parser::ParseWhileStat() {
	lexer_->MatchToken(TokenType::kKwWhile);
	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepRParen);
	auto block = ParseBlockStat();
	return std::make_unique<WhileStat>(std::move(exp), std::move(block));
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
	return std::make_unique<ReturnStat>(std::move(exp));
}

std::unique_ptr<TryStat> Parser::ParseTryStat() {
	lexer_->MatchToken(TokenType::kKwTry);

	auto block = ParseBlockStat();

	auto token = lexer_->PeekToken();

	std::unique_ptr<CatchStat> catch_stat;
	if (token.Is(TokenType::kKwCatch)) {
		catch_stat = ParseCatchStat();
		token = lexer_->PeekToken();
	}

	std::unique_ptr<FinallyStat> finally_stat;
	if (token.Is(TokenType::kKwFinally)) {
		finally_stat = ParseFinallyStat();
	}

	return std::make_unique<TryStat>(std::move(block), std::move(catch_stat), std::move(finally_stat));
}

std::unique_ptr<CatchStat> Parser::ParseCatchStat() {
	lexer_->MatchToken(TokenType::kKwCatch);

	lexer_->MatchToken(TokenType::kSepLParen);
	auto exp = ParseIdentifierExp();
	lexer_->MatchToken(TokenType::kSepRParen);

	auto block = ParseBlockStat();

	return std::make_unique<CatchStat>(std::move(exp), std::move(block));
}

std::unique_ptr<FinallyStat> Parser::ParseFinallyStat() {
	lexer_->MatchToken(TokenType::kKwFinally);
	auto block = ParseBlockStat();
	return std::make_unique<FinallyStat>(std::move(block));
}

std::unique_ptr<ThrowStat> Parser::ParseThrowStat() {
	lexer_->MatchToken(TokenType::kKwThrow);
	auto exp = ParseExp();
	return std::make_unique<ThrowStat>(std::move(exp));
}


std::unique_ptr<NewVarStat> Parser::ParseNewVarStat() {
	lexer_->MatchToken(TokenType::kKwLet);
	auto var_name = lexer_->MatchToken(TokenType::kIdentifier).str();
	lexer_->MatchToken(TokenType::kOpAssign);
	auto exp = ParseExp();
	lexer_->MatchToken(TokenType::kSepSemi);
	return  std::make_unique<NewVarStat>(var_name, std::move(exp));
}


std::unique_ptr<Exp> Parser::ParseExp() {
	return ParseExp20();
}

std::unique_ptr<Exp> Parser::ParseExp20() {
	auto exp = ParseExp19();
	do {
		auto type = lexer_->PeekToken().type();
		if (type != TokenType::kSepComma) {
			break;
		}
		lexer_->NextToken();
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp19());
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp19() {
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwYield) {
		lexer_->NextToken();

		std::unique_ptr<Exp> yielded_value = ParseExp18();
		return std::make_unique<YieldExp>(std::move(yielded_value));
	}

	return ParseExp18();
}


std::unique_ptr<Exp> Parser::ParseExp18() {
	// 赋值，右结合
	auto exp = ParseExp17();
	auto type = lexer_->PeekToken().type();
	if (type != TokenType::kOpAssign) {
		return exp;
	}
	lexer_->NextToken();
	exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp18());
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
	exp = std::make_unique<TernaryOpExp>(TokenType::kOpTernary, std::move(exp), std::move(exp2), std::move(exp3));
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp15());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp14());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp13());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp12());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp11());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp10());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp9());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp8());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp7());
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
		exp = std::make_unique<BinaryOpExp>(std::move(exp), type, ParseExp6());
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
		exp = std::make_unique<UnaryOpExp>(TokenType::kOpSuffixInc, std::move(exp));
	} while (true);
	return exp;
}

std::unique_ptr<Exp> Parser::ParseNewExp() {
	lexer_->NextToken();
	std::unique_ptr<Exp> callee;

	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		callee = ParseNewExp();
	}
	else {
		callee = ParseExp2(false);
	}

	std::vector<std::unique_ptr<Exp>> args;
	if (lexer_->PeekToken().Is(TokenType::kSepLParen)) {
		args = ParseExpList(TokenType::kSepLParen, TokenType::kSepRParen, false);
	}
	auto exp = std::make_unique<NewExp>(std::move(callee), std::move(args));

	return exp;
}

std::unique_ptr<Exp> Parser::ParseExp3() {
	auto type = lexer_->PeekToken().type();
	if (type == TokenType::kKwNew) {
		return ParseNewExp();
	}
	return ParseExp2(true);
}


std::unique_ptr<Exp> Parser::ParseExp2(bool match_lparen) {
	auto exp = ParsePrimaryExp(); // 解析基本表达式（标识符、字面量等）
	do {
		auto type = lexer_->PeekToken().type();
		if (type == TokenType::kSepDot) {
			lexer_->NextToken();
			auto member = ParsePrimaryExp();

			bool is_method_call = false;
			if (lexer_->PeekToken().type() == TokenType::kSepLParen) {
				is_method_call = true;
			}
			exp = std::make_unique<MemberExp>(std::move(exp), std::move(member), is_method_call);
		}
		else if (type == TokenType::kSepLBrack) {
			lexer_->NextToken();
			auto index = ParseExp();
			lexer_->MatchToken(TokenType::kSepRBrack);

			bool is_method_call = false;
			if (lexer_->PeekToken().type() == TokenType::kSepLParen) {
				is_method_call = true;
			}
			exp = std::make_unique<IndexedExp>(std::move(exp), std::move(index), is_method_call);
		}
		else if (match_lparen && type == TokenType::kSepLParen) {
			auto args = ParseExpList(TokenType::kSepLParen, TokenType::kSepRParen, false);
			exp = std::make_unique<FunctionCallExp>(std::move(exp), std::move(args));
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
		return ParsePrimaryExp();
	}
}


std::unique_ptr<IdentifierExp> Parser::ParseIdentifierExp() {
	auto token = lexer_->MatchToken(TokenType::kIdentifier);
	auto exp = std::make_unique<IdentifierExp>(token.str());
	exp->value_category = ExpValueCategory::kLeftValue;
	return exp;
}

std::unique_ptr<Exp> Parser::ParsePrimaryExp() {
	auto token = lexer_->PeekToken();
	std::unique_ptr<Exp> exp;
	switch (token.type()) {
	case TokenType::kKwAsync:
	case TokenType::kKwFunction:
		exp = ParseFunctionDeclExp();
		break;
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
				// ParseExp20会解析kSepComma，避免
				obj_literal.emplace(ident.str(), ParseExp19());
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
		exp = ParseIdentifierExp();
		break;
	case TokenType::kKwThis: {
		lexer_->NextToken();
		exp = std::make_unique<ThisExp>();
		break;
	}
	}
	}

	if (!exp) {
		throw ParserException("Unable to parse expression.");
	}
	return exp;
}

std::unique_ptr<FuncDeclExp> Parser::ParseFunctionDeclExp() {
	FunctionType type = FunctionType::kNormal;
	if (lexer_->PeekToken().Is(TokenType::kKwAsync)) {
		lexer_->NextToken();
		type = FunctionType::kAsync;
	}
	lexer_->MatchToken(TokenType::kKwFunction);
	if (lexer_->PeekToken().Is(TokenType::kOpMul)) {
		if (type != FunctionType::kNormal) {
			throw ParserException("Does not support asynchronous generator function.");
		}
		// 生成器函数
		lexer_->NextToken();
		type = FunctionType::kGenerator;
	}
	std::string func_name;
	if (lexer_->PeekToken().Is(TokenType::kIdentifier)) {
		func_name = lexer_->NextToken().str();
	}
	auto par_list = ParseParNameList();
	auto block = ParseBlockStat();
	return std::make_unique<FuncDeclExp>(func_name, par_list, std::move(block), type);
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
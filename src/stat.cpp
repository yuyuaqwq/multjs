#include "stat.h"

namespace mjs {

StatType ExpStat::GetType() const noexcept {
	return StatType::kExp;
}

StatType BlockStat::GetType() const noexcept {
	return StatType::kBlock;
}

BlockStat::BlockStat(std::vector<std::unique_ptr<Stat>>&& stat_list)
	: stat_list(std::move(stat_list)) {}


ExpStat::ExpStat(std::unique_ptr<Exp> exp)
	: exp(std::move(exp)){}


StatType IfStat::GetType() const noexcept {
	return StatType::kIf;
}

IfStat::IfStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block, std::vector<std::unique_ptr<ElseIfStat>>&& else_if_stat_list, std::unique_ptr<ElseStat> else_stat)
	: exp(std::move(exp))
	, block(std::move(block))
	, else_if_stat_list(std::move(else_if_stat_list))
	, else_stat(std::move(else_stat)) {}


StatType ElseIfStat::GetType() const noexcept {
	return StatType::kElseIf;
}

ElseIfStat::ElseIfStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block)
	: exp(std::move(exp))
	, block(std::move(block)) {}


StatType ElseStat::GetType() const noexcept {
	return StatType::kElse;
}

ElseStat::ElseStat(std::unique_ptr<BlockStat> block) :
	block(std::move(block)) {
}


StatType ForStat::GetType() const noexcept {
	return StatType::kFor;
}

ForStat::ForStat(const std::string& var_name, std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block)
	: var_name(var_name)
	, exp(std::move(exp))
	, block(std::move(block)) {}

StatType WhileStat::GetType() const noexcept {
	return StatType::kWhile;
}

WhileStat::WhileStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block) :
	exp(std::move(exp)), block(std::move(block)) {
}

StatType ContinueStat::GetType() const noexcept {
	return StatType::kContinue;
}

StatType BreakStat::GetType() const noexcept {
	return StatType::kBreak;
}

ReturnStat::ReturnStat(std::unique_ptr<Exp> exp)
	: exp(std::move(exp)) {}

StatType ReturnStat::GetType() const noexcept {
	return StatType::kReturn;
}


StatType NewVarStat::GetType() const noexcept {
	return StatType::kNewVar;
}

NewVarStat::NewVarStat(const std::string& var_name, std::unique_ptr<Exp> exp)
	: var_name(var_name)
	, exp(std::move(exp)) {}


} // namespace mjs
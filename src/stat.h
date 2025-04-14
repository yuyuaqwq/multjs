#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/function_def.h>
#include <mjs/class_def.h>

#include "token.h"
#include "exp.h"

namespace mjs {

enum class StatType {
	kExp,
	kIf,
	kElseIf,
	kElse,
	kFor,
	kWhile,
	kContinue,
	kBreak,
	kReturn,
	kTry,
	kCatch,
	kFinally,
	kThrow,
	kNewVar,
	kBlock,
};

struct Stat {
	virtual StatType GetType() const noexcept = 0;
};

struct BlockStat : public Stat {
	virtual StatType GetType() const noexcept;
	BlockStat(std::vector<std::unique_ptr<Stat>>&& stat_list);

	std::vector<std::unique_ptr<Stat>> stat_list;
};

struct ExpStat : public Stat {
	virtual StatType GetType() const noexcept;
	ExpStat(std::unique_ptr<Exp> exp);

	std::unique_ptr<Exp> exp;
};

struct ElseIfStat;
struct ElseStat;
struct IfStat : public Stat {
	virtual StatType GetType() const noexcept;
	IfStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block, std::vector<std::unique_ptr<ElseIfStat>>&& else_if_stat_list, std::unique_ptr<ElseStat> else_stat);

	std::unique_ptr<Exp> exp;
	std::unique_ptr<BlockStat> block;
	std::vector<std::unique_ptr<ElseIfStat>> else_if_stat_list;
	std::unique_ptr<ElseStat> else_stat;
};


struct ElseIfStat : public Stat {
	virtual StatType GetType() const noexcept;
	ElseIfStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block);

	std::unique_ptr<Exp> exp;
	std::unique_ptr<BlockStat> block;
};


struct ElseStat : public Stat {
	virtual StatType GetType() const noexcept;
	ElseStat(std::unique_ptr<BlockStat> t_block);

	std::unique_ptr<BlockStat> block;
};


struct ForStat : public Stat {
	virtual StatType GetType() const noexcept;
	ForStat(const std::string& var_name, std::unique_ptr<Exp> exp,std::unique_ptr<BlockStat> block);

	std::string var_name;
	std::unique_ptr<Exp> exp;
	std::unique_ptr<BlockStat> block;
};


struct WhileStat : public Stat {
	virtual StatType GetType() const noexcept;
	WhileStat(std::unique_ptr<Exp> exp, std::unique_ptr<BlockStat> block);

	std::unique_ptr<Exp> exp;
	std::unique_ptr<BlockStat> block;
};


struct ContinueStat : public Stat {
	virtual StatType GetType() const noexcept;
};


struct BreakStat : public Stat {
	virtual StatType GetType() const noexcept;
};


struct ReturnStat : public Stat {
	virtual StatType GetType() const noexcept;
	ReturnStat(std::unique_ptr<Exp> exp);

	std::unique_ptr<Exp> exp;
};

struct CatchStat : public Stat {
	CatchStat(std::unique_ptr<IdentifierExp> exp, std::unique_ptr<BlockStat> block)
		: exp(std::move(exp))
		, block(std::move(block)) {}

	virtual StatType GetType() const noexcept override {
		return StatType::kCatch;
	}

	std::unique_ptr<IdentifierExp> exp;
	std::unique_ptr<BlockStat> block;
};

struct FinallyStat : public Stat {
	FinallyStat(std::unique_ptr<BlockStat> block)
		: block(std::move(block)) {}

	virtual StatType GetType() const noexcept override {
		return StatType::kFinally;
	}

	std::unique_ptr<BlockStat> block;
};

struct TryStat : public Stat {
	TryStat(std::unique_ptr<BlockStat> block, std::unique_ptr<CatchStat> catch_stat
		, std::unique_ptr<FinallyStat> finally_stat)
		: block(std::move(block))
		, catch_stat(std::move(catch_stat))
		, finally_stat(std::move(finally_stat)) {}

	virtual StatType GetType() const noexcept override {
		return StatType::kTry;
	}

	std::unique_ptr<BlockStat> block;
	std::unique_ptr<CatchStat> catch_stat;
	std::unique_ptr<FinallyStat> finally_stat;
};

struct ThrowStat : public Stat {
	ThrowStat(std::unique_ptr<Exp> exp)
		: exp(std::move(exp)) {}

	virtual StatType GetType() const noexcept override {
		return StatType::kThrow;
	}

	std::unique_ptr<Exp> exp;
};

struct NewVarStat : public Stat {
	virtual StatType GetType() const noexcept;
	NewVarStat(const std::string& var_name, std::unique_ptr<Exp> exp, TokenType keyword_type);

	std::string var_name;
	std::unique_ptr<Exp> exp;
	TokenType keyword_type;
};

} // namespace mjs
#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/function_def.h>
#include <mjs/class_def/class_def.h>

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
	kLabel,
	kBlock,
	kImport,
	kExport,
};

struct Stat {
	virtual StatType GetType() const noexcept = 0;

	template<typename StatT>
	StatT& get() {
		return *static_cast<StatT*>(this);
	}

	template<typename StatT>
	const StatT& get() const {
		return *static_cast<const StatT*>(this);
	}
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

	ContinueStat(std::optional<std::string> label_name)
		: label_name(std::move(label_name)) {}

	std::optional<std::string> label_name;
};


struct BreakStat : public Stat {
	virtual StatType GetType() const noexcept;

	BreakStat(std::optional<std::string> label_name)
		: label_name(std::move(label_name)) {}

	std::optional<std::string> label_name;
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

struct LabelStat : public Stat {
	virtual StatType GetType() const noexcept {
		return StatType::kLabel;
	}
	LabelStat(std::string label_name, std::unique_ptr<Stat> stat)
		: label_name(std::move(label_name)) 
		, stat(std::move(stat)) {}

	std::string label_name;
	std::unique_ptr<Stat> stat;
};

struct NewVarStat : public Stat {
	virtual StatType GetType() const noexcept;
	NewVarStat(std::string var_name, std::unique_ptr<Exp> exp, TokenType keyword_type);

	std::string var_name;
	std::unique_ptr<Exp> exp;
	TokenType keyword_type;

	struct {
		uint32_t is_export = 1;
	} flags;
};


struct ImportStat : public Stat {
	virtual StatType GetType() const noexcept {
		return StatType::kImport;
	}
	ImportStat(std::string path, std::string var_name)
		: path(std::move(path))
		, var_name(std::move(var_name)) {}

	std::string path;
	std::string var_name;
};

struct ExportStat : public Stat {
	virtual StatType GetType() const noexcept {
		return StatType::kExport;
	}
	ExportStat(std::unique_ptr<Stat> stat)
		: stat(std::move(stat)) {}

	std::unique_ptr<Stat> stat;
};

} // namespace mjs
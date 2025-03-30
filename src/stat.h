#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/function_def.h>

#include "token.h"
#include "exp.h"

namespace mjs {

enum class StatType {
	kExp,
	kFunctionDecl,
	kIf,
	kElseIf,
	kElse,
	kFor,
	kWhile,
	kContinue,
	kBreak,
	kReturn,
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

struct FuncDeclStat : public Stat {
	virtual StatType GetType() const noexcept;
	FuncDeclStat(const std::string& func_name, const std::vector<std::string>& par_list
		, std::unique_ptr<BlockStat> block, FuncType func_type);

	std::string func_name;
	std::vector<std::string> par_list;
	std::unique_ptr<BlockStat> block;

	FuncType func_type;
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


struct NewVarStat : public Stat {
	virtual StatType GetType() const noexcept;
	NewVarStat(const std::string& var_name, std::unique_ptr<Exp> t_exp);

	std::string var_name;
	std::unique_ptr<Exp> exp;
};

} // namespace mjs
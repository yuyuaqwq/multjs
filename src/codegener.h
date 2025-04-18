#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/object/function_object.h>

#include "scope.h"
#include "stat.h"
#include "exp.h"

namespace mjs {

// 代码生成时发生的异常
class CodeGenerException : public std::exception{
public:
	using Base = std::exception;
	using Base::Base;
};

class Runtime;
class CodeGener : public noncopyable {
public:
	CodeGener(Runtime* runtime);

	void RegisterCppFunction(const std::string& func_name, CppFunction func);
	Value Generate(BlockStat* block);

private:
	void EntryScope(FunctionDef* sub_func = nullptr, ScopeType type = ScopeType::kNone);
	void ExitScope();

	ConstIndex AllocConst(Value&& value);
	const Value& GetConstValueByIndex(ConstIndex idx);

	const VarInfo& AllocVar(const std::string& name, VarFlags flags = VarFlags::kNone);
	const VarInfo* FindVarIndexByName(const std::string& name);
	bool IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types);
	
	const VarInfo& GetVarByExp(Exp* exp);

	void GenerateBlock(BlockStat* block, bool entry_scope = true, ScopeType type = ScopeType::kNone);
	void GenerateStat(Stat* stat);
	void GenerateReturnStat(ReturnStat* stat);
	void GenerateNewVarStat(NewVarStat* stat);
	void GenerateIfStat(IfStat* stat);
	void GenerateWhileStat(WhileStat* stat);
	void GenerateContinueStat(ContinueStat* stat);
	void GenerateBreakStat(BreakStat* stat);

	void GenerateTryStat(TryStat* stat);
	void GenerateThrowStat(ThrowStat* stat);

	void GenerateExp(Exp* exp);
	void GenerateIfEq(Exp* exp);

	void GenerateFunctionDeclExp(FuncDeclExp* exp);

	Value MakeValue(Exp* exp);
	void GenerateParList(const std::vector<std::unique_ptr<Exp>>& par_list);

private:
	Runtime* runtime_;

	// 函数
	FunctionDef* cur_func_def_ = nullptr;				// 当前生成函数

	// 作用域
	std::vector<Scope> scopes_;

	// 循环
	uint32_t cur_loop_start_pc_ = 0;
	std::vector<uint32_t>* cur_loop_repair_end_pc_list_ = nullptr;

	// 异常
	bool has_finally_ = false;  // 当前作用域是否关联finally块
};

} // namespace mjs
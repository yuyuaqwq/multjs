#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/function_object.h>

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
	void EntryScope(FunctionDef* sub_func = nullptr);
	void ExitScope();

	ConstIndex AllocConst(Value&& value);
	const Value& FindConstValueByIndex(ConstIndex idx);

	VarIndex AllocVar(const std::string& name);
	std::optional<VarIndex> FindVarIndexByName(const std::string& name);
	VarIndex GetVarByExp(Exp* exp);

	void GenerateBlock(BlockStat* block);
	void GenerateStat(Stat* stat);
	void GenerateFunctionDeclStat(FuncDeclStat* stat);
	void GenerateReturnStat(ReturnStat* stat);
	void GenerateNewVarStat(NewVarStat* stat);
	void GenerateIfStat(IfStat* stat);
	void GenerateWhileStat(WhileStat* stat);
	void GenerateContinueStat(ContinueStat* stat);
	void GenerateBreakStat(BreakStat* stat);
	void GenerateExp(Exp* exp);
	void GenerateIfEq(Exp* exp);

	Value MakeValue(Exp* exp);
	void GenerateFunctionCallPar(FunctionCallExp* func_call_exp);

private:
	Runtime* runtime_;

	// 函数
	FunctionDef* cur_func_def_ = nullptr;				// 当前生成函数

	// 作用域
	std::vector<Scope> scopes_;

	// 循环
	uint32_t cur_loop_start_pc_ = 0;
	std::vector<uint32_t>* cur_loop_repair_end_pc_list_ = nullptr;
};

} // namespace mjs
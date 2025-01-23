#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "instr.h"
#include "vm.h"

#include "value.h"
#include "stack_frame.h"
#include "const_pool.h"
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
class CodeGener {
public:
	CodeGener(Runtime* runtime);

	void RegistryFunctionBridge(std::string func_name, FunctionBridgeObject func);
	Value Generate(BlockStat* block);

private:
	void EntryScope(FunctionBodyObject* sub_func = nullptr);
	void ExitScope();
	uint32_t AllocConst(Value&& value);
	uint32_t AllocVar(std::string varName);
	uint32_t GetVar(std::string varName);

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

private:
	Runtime* runtime_;

	// 函数
	FunctionBodyObject* cur_func_ = nullptr;				// 当前生成函数

	// 作用域
	std::vector<Scope> scopes_;

	// 循环
	uint32_t cur_loop_start_pc_ = 0;
	std::vector<uint32_t>* cur_loop_repair_end_pc_list_ = nullptr;
};

} // namespace mjs
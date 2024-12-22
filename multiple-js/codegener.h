#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "instr.h"
#include "vm.h"

#include "value.h"
#include "section.h"

#include "stat.h"
#include "exp.h"

namespace mjs {

struct Scope {
	FunctionBodyValue* func;						// 作用域所属函数
	uint32_t var_count;								// 当前函数在当前作用域中的有效变量计数
	std::unordered_map<std::string, uint32_t> var_table;		// 变量表，key为变量索引
};

// 代码生成时发生的异常
class CodeGenerException : public std::exception{
public:
	using Base = std::exception;
	using Base::Base;
};


class CodeGener {
public:
	CodeGener(ValueSection* const_sect);

	void EntryScope(FunctionBodyValue* sub_func = nullptr);
	void ExitScope();
	uint32_t AllocConst(std::unique_ptr<Value> value);
	uint32_t AllocVar(std::string varName);
	uint32_t GetVar(std::string varName);
	void RegistryFunctionBridge(std::string func_name, FunctionBridgeCall func_addr);

	void Generate(BlockStat* block, ValueSection* const_sect);
	void GenerateBlock(BlockStat* block);
	void GenerateStat(Stat* stat);
	void GenerateFunctionDeclStat(FuncDeclStat* stat);
	void GenerateReturnStat(ReturnStat* stat);
	void GenerateNewVarStat(NewVarStat* stat);
	void GenerateAssignStat(AssignStat* stat);
	void GenerateIfStat(IfStat* stat);
	void GenerateWhileStat(WhileStat* stat);
	void GenerateContinueStat(ContinueStat* stat);
	void GenerateBreakStat(BreakStat* stat);
	void GenerateExp(Exp* exp);

private:
	// 函数
	FunctionBodyValue* cur_func_ = nullptr;				// 当前生成函数

	// 常量
	std::unordered_map<Value*, uint32_t> const_map_;		// 暂时有问题，指针就没办法找重载<了
	ValueSection* const_sect_;					// 全局常量区

	// 作用域
	std::vector<Scope> scope_;

	// 循环
	uint32_t cur_loop_start_pc_ = 0;
	std::vector<uint32_t>* cur_loop_repair_end_pc_list_ = nullptr;
};

} // namespace mjs
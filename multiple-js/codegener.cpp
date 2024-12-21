#include "codegener.h"


namespace mjs {

CodeGener::CodeGener(ValueSection* const_sect_)
	: const_sect_(const_sect_)
{
	const_sect_->Clear();
	const_sect_->Push(std::make_unique<FunctionBodyValue>(0));
	cur_func_ = const_sect_->Get(0)->GetFunctionBody();

	scope_.push_back(Scope{ cur_func_ });
}

void CodeGener::EntryScope(FunctionBodyValue* subFunc) {
	if (!subFunc) {
		// 进入的作用域不是新函数
		scope_.push_back(Scope{ cur_func_, scope_[scope_.size() - 1].var_count });
		return;
	}
	// 进入的作用域是新函数
	scope_.push_back(Scope{ subFunc, 0 });
}

void CodeGener::ExitScope() {
	scope_.pop_back();
}

uint32_t CodeGener::AllocConst(std::unique_ptr<Value> value) {
	uint32_t constIdx;
	auto it = const_map_.find(value.get());
	if (it == const_map_.end()) {
		const_sect_->Push(std::move(value));
		constIdx = const_sect_->Size() - 1;
	}
	else {
		constIdx = it->second;
	}
	return constIdx;
}

uint32_t CodeGener::AllocVar(std::string var_name) {
	auto& varTable = scope_[scope_.size() - 1].var_table;
	if (varTable.find(var_name) != varTable.end()) {
		throw CodeGenerException("local var redefinition");
	}
	auto varIdx = scope_[scope_.size() - 1].var_count++;
	varTable.insert(std::make_pair(var_name, varIdx));
	return varIdx;
}

uint32_t CodeGener::GetVar(std::string var_name) {
	uint32_t varIdx = -1;
	// 就近找变量

	for (int i = scope_.size() - 1; i >= 0; i--) {
		auto& varTable = scope_[i].var_table;
		auto it = varTable.find(var_name);
		if (it != varTable.end()) {
			if (scope_[i].func == cur_func_) {
				varIdx = it->second;
			}
			else {
				// 引用外部函数的变量，需要捕获，为当前函数加载upvalue变量
				auto constIdx = AllocConst(std::make_unique<UpValue>(it->second, scope_[i].func));
				cur_func_->instr_sect.EmitPushK(constIdx);
				varIdx = AllocVar(var_name);
				cur_func_->instr_sect.EmitPopV(varIdx);
			}
			break;
		}
	}
	return varIdx;
}


void CodeGener::RegistryFunctionBridge(std::string func_name, FunctionBridgeCall funcAddr) {
	auto varIdx = AllocVar(func_name);
	auto constIdx = AllocConst(std::make_unique<FunctionBridgeValue>(funcAddr));


	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_->instr_sect.EmitPushK(constIdx);
	cur_func_->instr_sect.EmitPopV(varIdx);
}


void CodeGener::Generate(BlockStat* block, ValueSection* constSect) {

	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	cur_func_->instr_sect.EmitStop();

}

void CodeGener::GenerateBlock(BlockStat* block) {
	EntryScope();
	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	ExitScope();
}

void CodeGener::GenerateStat(Stat* stat) {
	switch (stat->GetType())
	{
	case StatType::kBlock: {
		GenerateBlock(static_cast<BlockStat*>(stat));
		break;
	}
	case StatType::kExp: {
		auto expStat = static_cast<ExpStat*>(stat)->exp.get();

		// 抛弃纯表达式语句的最终结果
		if (expStat) {
			GenerateExp(expStat);
			cur_func_->instr_sect.EmitPop();
		}

		break;
	}
	case StatType::kFuncDef: {
		GenerateFuncDefStat(static_cast<FuncDefStat*>(stat));
		break;
	}
	case StatType::kReturn: {
		GenerateReturnStat(static_cast<ReturnStat*>(stat));
		break;
	}
	case StatType::kAssign: {
		GenerateAssignStat(static_cast<AssignStat*>(stat));
		break;
	}
	case StatType::kNewVar: {
		GenerateNewVarStat(static_cast<NewVarStat*>(stat));
		break;
	}
	case StatType::kIf: {
		GenerateIfStat(static_cast<IfStat*>(stat));
		break;
	}
	case StatType::kWhile: {
		GenerateWhileStat(static_cast<WhileStat*>(stat));
		break;
	}
	case StatType::kContinue: {
		GenerateContinueStat(static_cast<ContinueStat*>(stat));
		break;
	}
	case StatType::kBreak: {
		GenerateBreakStat(static_cast<BreakStat*>(stat));
		break;
	}

	default:
		throw CodeGenerException("Unknown statement type");
	}
}

void CodeGener::GenerateFuncDefStat(FuncDefStat* stat) {
	auto varIdx = AllocVar(stat->func_name);
	auto constIdx = AllocConst(std::make_unique<FunctionBodyValue>(stat->par_list.size()));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_->instr_sect.EmitPushK(constIdx);
	cur_func_->instr_sect.EmitPopV(varIdx);


	// 保存环境，以生成新指令流
	auto savefunc = cur_func_;

	// 切换环境
	EntryScope(const_sect_->Get(constIdx)->GetFunctionBody());
	cur_func_ = const_sect_->Get(constIdx)->GetFunctionBody();


	for (int i = 0; i < cur_func_->par_count; i++) {
		AllocVar(stat->par_list[i]);
	}

	auto block = stat->block.get();
	for (int i = 0; i < block->stat_list.size(); i++) {
		auto& stat = block->stat_list[i];
		GenerateStat(stat.get());
		if (i == block->stat_list.size() - 1) {
			if (stat->GetType() != StatType::kReturn) {
				// 补全末尾的return
				cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NullValue>()));
				cur_func_->instr_sect.EmitRet();
			}
		}
	}

	// 恢复环境
	ExitScope();
	cur_func_ = savefunc;
}

void CodeGener::GenerateReturnStat(ReturnStat* stat) {
	if (stat->exp.get()) {
		GenerateExp(stat->exp.get());
	}
	else {
		cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NullValue>()));
	}
	cur_func_->instr_sect.EmitRet();
}

// 为了简单起见，不提前计算/最后修复局部变量的总数，因此不能分配到栈上
// 解决方案：
	// 另外提供变量表容器

// 函数内指令流的变量索引在生成时，无法确定变量分配的索引
// 原因：
	// 定义时无法提前得知call的位置，且call可能有多处，每次call时，变量表的状态可能都不一样
// 解决方案：
	// 为每个函数提供一个自己的变量表，不放到虚拟机中，call时切换变量表
	// 在代码生成过程中，需要获取变量时，如果发现使用的变量是当前函数之外的外部作用域的，就会在常量区中创建一个类型为upvalue的变量，并加载到当前函数的变量中
	// upvalue存储了外部函数的Body地址，以及对应的变量索引


void CodeGener::GenerateNewVarStat(NewVarStat* stat) {
	auto varIdx = AllocVar(stat->var_name);
	GenerateExp(stat->exp.get());		// 生成表达式计算指令，最终结果会到栈顶
	cur_func_->instr_sect.EmitPopV(varIdx);	// 弹出到局部变量中
}

void CodeGener::GenerateAssignStat(AssignStat* stat) {
	auto varIdx = GetVar(stat->var_name);
	if (varIdx == -1) {
		throw CodeGenerException("var not defined");
	}
	GenerateExp(stat->exp.get());		// 表达式压栈
	cur_func_->instr_sect.EmitPopV(varIdx);	// 弹出到变量中
}

void CodeGener::GenerateIfStat(IfStat* stat) {
	GenerateExp(stat->exp.get());		// 表达式结果压栈


	uint32_t ifPc = cur_func_->instr_sect.GetPc() + 1;		// 留给下一个elif/else修复
	cur_func_->instr_sect.EmitJcf(0);		// 提前写入条件为false时跳转的指令

	GenerateBlock(stat->block.get());



	// if

	// jcf end
	// ...
// end:
	// ...


	// if
	// else

	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....


	// if 
	// elif
	// else

	// jcf elif
	// ...
	// jmp end
// elif:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....


	// if 
	// elif
	// elif
	// else

	// jcf elif1
	// ...
	// jmp end
// elif1:
	// jcf elif2
	// ...
	// jmp end
// elif2:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....

	std::vector<uint32_t> repairEndPcList;
	for (auto& elifStat : stat->else_if_stat_list) {
		repairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
		cur_func_->instr_sect.EmitJmp(0);		// 提前写入上一分支退出if分支结构的jmp跳转

		// 修复条件为false时，跳转到if/elif块之后的地址
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();

		GenerateExp(elifStat->exp.get());		// 表达式结果压栈
		ifPc = cur_func_->instr_sect.GetPc() + 1;		// 留给下一个elif/else修复
		cur_func_->instr_sect.EmitJcf(0);		// 提前写入条件为false时跳转的指令

		GenerateBlock(elifStat->block.get());
	}

	if (stat->else_stat.get()) {
		repairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
		cur_func_->instr_sect.EmitJmp(0);		// 提前写入上一分支退出if分支结构的jmp跳转

		// 修复条件为false时，跳转到if/elif块之后的地址
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();

		GenerateBlock(stat->else_stat->block.get());
	}
	else {
		// 修复条件为false时，跳转到if/elif块之后的地址
		*(uint32_t*)cur_func_->instr_sect.GetPtr(ifPc) = cur_func_->instr_sect.GetPc();
	}

	// 至此if分支结构结束，修复所有退出分支结构的地址
	for (auto repairEndPc : repairEndPcList) {
		*(uint32_t*)cur_func_->instr_sect.GetPtr(repairEndPc) = cur_func_->instr_sect.GetPc();
	}
}

void CodeGener::GenerateWhileStat(WhileStat* stat) {
	auto save_cur_loop_repair_end_pc_list = cur_loop_repair_end_pc_list_;
	auto save_cur_loop_start_pc = cur_loop_start_pc_;

	std::vector<uint32_t> loopRepairEndPcList;
	cur_loop_repair_end_pc_list_ = &loopRepairEndPcList;

	// 记录重新循环的pc
	uint32_t loop_start_pc = cur_func_->instr_sect.GetPc();
	cur_loop_start_pc_ = loop_start_pc;

	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 等待修复
	loopRepairEndPcList.push_back(cur_func_->instr_sect.GetPc() + 1);
	// 提前写入条件为false时跳出循环的指令
	cur_func_->instr_sect.EmitJcf(0);

	GenerateBlock(stat->block.get());

	// 重新回去看是否需要循环
	cur_func_->instr_sect.EmitJmp(loop_start_pc);

	for (auto repairEndPc : loopRepairEndPcList) {
		// 修复跳出循环的指令的pc
		*(uint32_t*)cur_func_->instr_sect.GetPtr(repairEndPc) = cur_func_->instr_sect.GetPc();
	}

	cur_loop_start_pc_ = save_cur_loop_start_pc;
	cur_loop_repair_end_pc_list_ = save_cur_loop_repair_end_pc_list;

}

void CodeGener::GenerateContinueStat(ContinueStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_func_->instr_sect.EmitJmp(cur_loop_start_pc_);		// 跳回当前循环的起始pc
}

void CodeGener::GenerateBreakStat(BreakStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_loop_repair_end_pc_list_->push_back(cur_func_->instr_sect.GetPc() + 1);
	cur_func_->instr_sect.EmitJmp(0);		// 无法提前得知结束pc，保存待修复pc，等待修复
}


void CodeGener::GenerateExp(Exp* exp) {
	switch (exp->GetType())
	{
	case ExpType::kNull: {
		auto constIdx = AllocConst(std::make_unique<NullValue>());
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kBool: {
		auto boolexp = static_cast<BoolExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<BoolValue>(boolexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kNumber: {
		auto numexp = static_cast<NumberExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<NumberValue>(numexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kString: {
		auto strexp = static_cast<StringExp*>(exp);
		auto constIdx = AllocConst(std::make_unique<StringValue>(strexp->value));
		cur_func_->instr_sect.EmitPushK(constIdx);
		break;
	}
	case ExpType::kName: {
		// 是取变量值的话，查找到对应的变量编号，将其入栈
		auto nameExp = static_cast<NameExp*>(exp);

		auto varIdx = GetVar(nameExp->name);
		if (varIdx == -1) {
			throw CodeGenerException("var not defined");
		}

		cur_func_->instr_sect.EmitPushV(varIdx);	// 从变量中获取


		break;
	}
	case ExpType::kBinaOp: {
		auto binaOpExp = static_cast<BinaOpExp*>(exp);

		// 左右表达式的值入栈
		GenerateExp(binaOpExp->left_exp.get());
		GenerateExp(binaOpExp->right_exp.get());

		// 生成运算指令
		switch (binaOpExp->oper) {
		case TokenType::kOpAdd:
			cur_func_->instr_sect.EmitAdd();
			break;
		case TokenType::kOpSub:
			cur_func_->instr_sect.EmitSub();
			break;
		case TokenType::kOpMul:
			cur_func_->instr_sect.EmitMul();
			break;
		case TokenType::kOpDiv:
			cur_func_->instr_sect.EmitDiv();
			break;
		case TokenType::kOpNe:
			cur_func_->instr_sect.EmitNe();
			break;
		case TokenType::kOpEq:
			cur_func_->instr_sect.EmitEq();
			break;
		case TokenType::kOpLt:
			cur_func_->instr_sect.EmitLt();
			break;
		case TokenType::kOpLe:
			cur_func_->instr_sect.EmitLe();
			break;
		case TokenType::kOpGt:
			cur_func_->instr_sect.EmitGt();
			break;
		case TokenType::kOpGe:
			cur_func_->instr_sect.EmitGe();
			break;
		default:
			throw CodeGenerException("Unrecognized binary operator");
		}
		break;
	}
	case ExpType::kFunctionCall: {
		auto funcCallExp = static_cast<FunctionCallExp*>(exp);

		auto varIdx = GetVar(funcCallExp->name);
		if (varIdx == -1) {
			throw CodeGenerException("Function not defined");
		}

		//if (funcCallExp->par_list.size() < const_table_[]->GetFunctionBody()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		for (int i = funcCallExp->par_list.size() - 1; i >= 0; i--) {
			// 参数逆序入栈，由call负责将栈中参数放到变量表中
			GenerateExp(funcCallExp->par_list[i].get());
		}

		cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NumberValue>(funcCallExp->par_list.size())));

		// 函数原型存放在变量表中
		cur_func_->instr_sect.EmitCall(varIdx);
		break;
	}
	default:
		break;
	}
}


} // namespace codegener
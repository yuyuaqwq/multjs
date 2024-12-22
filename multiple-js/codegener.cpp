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

void CodeGener::EntryScope(FunctionBodyValue* sub_func) {
	if (!sub_func) {
		// 进入的作用域不是新函数
		scope_.push_back(Scope{ cur_func_, scope_.back().var_count });
		return;
	}
	// 进入的作用域是新函数
	scope_.push_back(Scope{ sub_func, 0 });
}

void CodeGener::ExitScope() {
	scope_.pop_back();
}

uint32_t CodeGener::AllocConst(std::unique_ptr<Value> value) {
	uint32_t const_idx;
	auto it = const_map_.find(value.get());
	if (it == const_map_.end()) {
		const_sect_->Push(std::move(value));
		const_idx = const_sect_->Size() - 1;
	}
	else {
		const_idx = it->second;
	}
	return const_idx;
}

uint32_t CodeGener::AllocVar(std::string var_name) {
	auto& var_table = scope_.back().var_table;
	if (var_table.find(var_name) != var_table.end()) {
		throw CodeGenerException("local var redefinition");
	}
	auto var_idx = scope_.back().var_count++;
	var_table.emplace(std::move(var_name), var_idx);
	return var_idx;
}

uint32_t CodeGener::GetVar(std::string var_name) {
	uint32_t var_idx = -1;
	// 就近找变量
	for (int i = scope_.size() - 1; i >= 0; i--) {
		auto& var_table = scope_[i].var_table;
		auto it = var_table.find(var_name);
		if (it != var_table.end()) {
			if (scope_[i].func == cur_func_) {
				var_idx = it->second;
			}
			else {
				// 引用外部函数的变量，需要捕获，为当前函数加载upvalue变量
				auto const_idx = AllocConst(std::make_unique<UpValue>(it->second, scope_[i].func));
				cur_func_->instr_sect.EmitLdc(const_idx);
				var_idx = AllocVar(var_name);
				cur_func_->instr_sect.EmitIStore(var_idx);
			}
			break;
		}
	}
	return var_idx;
}


void CodeGener::RegistryFunctionBridge(std::string func_name, FunctionBridgeCall func_addr) {
	auto var_idx = AllocVar(func_name);
	auto const_idx = AllocConst(std::make_unique<FunctionBridgeValue>(func_addr));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_->instr_sect.EmitLdc(const_idx);
	cur_func_->instr_sect.EmitIStore(var_idx);
}


void CodeGener::Generate(BlockStat* block, ValueSection* constSect) {
	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	// cur_func_->instr_sect.EmitStop();
}

void CodeGener::GenerateBlock(BlockStat* block) {
	EntryScope();
	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	ExitScope();
}

void CodeGener::GenerateStat(Stat* stat) {
	switch (stat->GetType()) {
	case StatType::kBlock: {
		GenerateBlock(static_cast<BlockStat*>(stat));
		break;
	}
	case StatType::kExp: {
		auto exp_stat = static_cast<ExpStat*>(stat)->exp.get();
		// 抛弃纯表达式语句的最终结果
		if (exp_stat) {
			GenerateExp(exp_stat);
			cur_func_->instr_sect.EmitPop();
		}
		break;
	}
	case StatType::kFunctionDecl: {
		GenerateFunctionDeclStat(static_cast<FuncDeclStat*>(stat));
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

void CodeGener::GenerateFunctionDeclStat(FuncDeclStat* stat) {
	auto var_idx = AllocVar(stat->func_name);
	auto const_idx = AllocConst(std::make_unique<FunctionBodyValue>(stat->par_list.size()));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_->instr_sect.EmitPushK(const_idx);
	cur_func_->instr_sect.EmitPopV(var_idx);

	// 保存环境，以生成新指令流
	auto savefunc = cur_func_;

	// 切换环境
	EntryScope(const_sect_->Get(const_idx)->GetFunctionBody());
	cur_func_ = const_sect_->Get(const_idx)->GetFunctionBody();

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
	auto var_idx = AllocVar(stat->var_name);
	GenerateExp(stat->exp.get());		// 生成表达式计算指令，最终结果会到栈顶
	cur_func_->instr_sect.EmitIStore(var_idx);	// 弹出到局部变量中
}

void CodeGener::GenerateAssignStat(AssignStat* stat) {
	auto varIdx = GetVar(stat->var_name);
	if (varIdx == -1) {
		throw CodeGenerException("var not defined");
	}
	// 表达式压栈
	GenerateExp(stat->exp.get());
	// 弹出到变量中
	cur_func_->instr_sect.EmitILoad(varIdx);
}

// 2字节指的是基于当前指令的offset
void CodeGener::GenerateIfStat(IfStat* stat) {
	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 留给下一个else if/else修复
	uint32_t if_pc = cur_func_->instr_sect.GetPc() + 1;
	// 提前写入条件为false时跳转的指令
	cur_func_->instr_sect.EmitOpcode(OpcodeType::kIfEq);
	cur_func_->instr_sect.EmitU16(0);
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
	// else if
	// else

	// jcf else if
	// ...
	// jmp end
// else if:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....


	// if 
	// else if
	// else if
	// else

	// jcf else if1
	// ...
	// jmp end
// else if1:
	// jcf elif2
	// ...
	// jmp end
// else if2:
	// jcf else
	// ...
	// jmp end
// else:
	// ...
// end:
	// ....

	std::vector<uint32_t> repair_end_pc_list;
	for (auto& else_if_stat : stat->else_if_stat_list) {
		repair_end_pc_list.push_back(cur_func_->instr_sect.GetPc() + 1);
		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_->instr_sect.EmitOpcode(OpcodeType::kGoto);
		cur_func_->instr_sect.EmitU16(0);

		// 修复条件为false时，跳转到if/elif块之后的地址
		*reinterpret_cast<uint16_t*>(cur_func_->instr_sect.GetPtr(if_pc)) = cur_func_->instr_sect.GetPc();

		// 表达式结果压栈
		GenerateExp(else_if_stat->exp.get());
		// 留给下一个else if/else修复
		if_pc = cur_func_->instr_sect.GetPc() + 1;
		// 提前写入条件为false时跳转的指令
		cur_func_->instr_sect.EmitOpcode(OpcodeType::kIfEq);
		cur_func_->instr_sect.EmitU16(0);

		GenerateBlock(else_if_stat->block.get());
	}

	if (stat->else_stat.get()) {
		repair_end_pc_list.push_back(cur_func_->instr_sect.GetPc() + 1);
		cur_func_->instr_sect.EmitOpcode(OpcodeType::kGoto);		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_->instr_sect.EmitU16(0);

		// 修复条件为false时，跳转到if/else if块之后的地址
		*reinterpret_cast<uint16_t*>(cur_func_->instr_sect.GetPtr(if_pc)) = cur_func_->instr_sect.GetPc();

		GenerateBlock(stat->else_stat->block.get());
	}
	else {
		// 修复条件为false时，跳转到if/else if块之后的地址
		*reinterpret_cast<uint16_t*>(cur_func_->instr_sect.GetPtr(if_pc)) = cur_func_->instr_sect.GetPc();
	}

	// 至此if分支结构结束，修复所有退出分支结构的地址
	for (auto repair_pnd_pc : repair_end_pc_list) {
		*reinterpret_cast<uint16_t*>(cur_func_->instr_sect.GetPtr(repair_pnd_pc)) = cur_func_->instr_sect.GetPc();
	}
}

void CodeGener::GenerateWhileStat(WhileStat* stat) {
	auto save_cur_loop_repair_end_pc_list = cur_loop_repair_end_pc_list_;
	auto save_cur_loop_start_pc = cur_loop_start_pc_;

	std::vector<uint32_t> loop_repair_end_pc_list;
	cur_loop_repair_end_pc_list_ = &loop_repair_end_pc_list;

	// 记录重新循环的pc
	uint32_t loop_start_pc = cur_func_->instr_sect.GetPc();
	cur_loop_start_pc_ = loop_start_pc;

	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 等待修复
	loop_repair_end_pc_list.push_back(cur_func_->instr_sect.GetPc() + 1);
	// 提前写入条件为false时跳出循环的指令
	cur_func_->instr_sect.EmitOpcode(OpcodeType::kIfEq);
	cur_func_->instr_sect.EmitU16(0);

	GenerateBlock(stat->block.get());

	// 重新回去看是否需要循环
	cur_func_->instr_sect.EmitOpcode(OpcodeType::kGoto);
	cur_func_->instr_sect.EmitU16(loop_start_pc);

	for (auto repair_end_pc : loop_repair_end_pc_list) {
		// 修复跳出循环的指令的pc
		*reinterpret_cast<uint16_t*>(cur_func_->instr_sect.GetPtr(repair_end_pc)) = cur_func_->instr_sect.GetPc();
	}

	cur_loop_start_pc_ = save_cur_loop_start_pc;
	cur_loop_repair_end_pc_list_ = save_cur_loop_repair_end_pc_list;
}

void CodeGener::GenerateContinueStat(ContinueStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	// 跳回当前循环的起始pc
	cur_func_->instr_sect.EmitOpcode(OpcodeType::kGoto);
	cur_func_->instr_sect.EmitU16(cur_loop_start_pc_);
}

void CodeGener::GenerateBreakStat(BreakStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_loop_repair_end_pc_list_->push_back(cur_func_->instr_sect.GetPc() + 1);
	// 无法提前得知结束pc，保存待修复pc，等待修复
	cur_func_->instr_sect.EmitOpcode(OpcodeType::kGoto);
	cur_func_->instr_sect.EmitU16(0);
}


void CodeGener::GenerateExp(Exp* exp) {
	switch (exp->GetType()) {
	case ExpType::kNull: {
		auto const_idx = AllocConst(std::make_unique<NullValue>());
		cur_func_->instr_sect.EmitPushK(const_idx);
		break;
	}
	case ExpType::kBool: {
		auto bool_exp = static_cast<BoolExp*>(exp);
		auto const_idx = AllocConst(std::make_unique<BoolValue>(bool_exp->value));
		cur_func_->instr_sect.EmitPushK(const_idx);
		break;
	}
	case ExpType::kNumber: {
		auto num_exp = static_cast<NumberExp*>(exp);
		auto value = num_exp->value;
		if (value >= 0 && value <= 5) {
			cur_func_->instr_sect.EmitOpcode(OpcodeType::kIConst_0 + value);
		}
		else if (value >= -128 && value < 128) {
			cur_func_->instr_sect.EmitOpcode(OpcodeType::kBIPush);
			cur_func_->instr_sect.EmitU8(value);
		}
		else if (value >= -32768 && value < 32768) {
			cur_func_->instr_sect.EmitOpcode(OpcodeType::kSIPush);
			cur_func_->instr_sect.EmitU16(value);
		}
		else {
			auto const_idx = AllocConst(std::make_unique<NumberValue>(value));
			cur_func_->instr_sect.EmitLdc(const_idx);
		}
		break;
	}
	case ExpType::kString: {
		auto str_exp = static_cast<StringExp*>(exp);
		auto const_idx = AllocConst(std::make_unique<StringValue>(str_exp->value));
		cur_func_->instr_sect.EmitPushK(const_idx);
		break;
	}
	case ExpType::kName: {
		// 是取变量值的话，查找到对应的变量编号，将其入栈
		auto name_exp = static_cast<NameExp*>(exp);
		auto var_idx = GetVar(name_exp->name);
		if (var_idx == -1) {
			throw CodeGenerException("var not defined");
		}
		cur_func_->instr_sect.EmitPushV(var_idx);	// 从变量中获取
		break;
	}
	case ExpType::kBinaOp: {
		auto bina_op_exp = static_cast<BinaOpExp*>(exp);

		// 左右表达式的值入栈
		GenerateExp(bina_op_exp->left_exp.get());
		GenerateExp(bina_op_exp->right_exp.get());

		// 生成运算指令
		switch (bina_op_exp->oper) {
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
		auto func_call_exp = static_cast<FunctionCallExp*>(exp);

		auto var_idx = GetVar(func_call_exp->name);
		if (var_idx == -1) {
			throw CodeGenerException("Function not defined");
		}

		//if (func_call_exp->par_list.size() < const_table_[]->GetFunctionBody()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		for (int i = func_call_exp->par_list.size() - 1; i >= 0; i--) {
			// 参数逆序入栈，由call负责将栈中参数放到变量表中
			GenerateExp(func_call_exp->par_list[i].get());
		}

		cur_func_->instr_sect.EmitPushK(AllocConst(std::make_unique<NumberValue>(func_call_exp->par_list.size())));

		// 函数原型存放在变量表中
		cur_func_->instr_sect.EmitCall(var_idx);
		break;
	}
	default:
		break;
	}
}

} // namespace mjs
#include "codegener.h"

#include <iostream>

#include <mjs/runtime.h>
#include <mjs/object/array_object.h>

namespace mjs {

CodeGener::CodeGener(Runtime* runtime)
	: runtime_(runtime) {}

void CodeGener::EntryScope(FunctionDef* sub_func, ScopeType type) {
	if (sub_func == nullptr) {
		sub_func = cur_func_def_;
	}
	scopes_.emplace_back(sub_func, type);
}

void CodeGener::ExitScope() {
	scopes_.pop_back();
}


ConstIndex CodeGener::AllocConst(Value&& value) {
	return runtime_->const_pool().insert(std::move(value));
}

const Value& CodeGener::GetConstValueByIndex(ConstIndex idx) {
	return runtime_->const_pool().at(idx);
}


const VarInfo& CodeGener::AllocVar(const std::string& var_name, VarFlags flags) {
	return scopes_.back().AllocVar(var_name, flags);
}

const VarInfo* CodeGener::FindVarIndexByName(const std::string& var_name) {
	const VarInfo* find_var_info = nullptr;;
	// 就近找变量
	for (ptrdiff_t i = scopes_.size() - 1; i >= 0; --i) {
		auto var_info = scopes_[i].FindVar(var_name);
		if (!var_info) {
			// 当前作用域找不到变量，向上层作用域找
			continue;
		}

		auto var_idx = var_info->var_idx;
		if (scopes_[i].function_def() == cur_func_def_) {
			find_var_info = var_info;
		}
		else {
			// 在上层函数作用域找到了，构建upvalue捕获链
			auto scope_func = scopes_[i].function_def();
			scope_func->AddClosureVar(var_idx, std::nullopt);

			for (size_t j = i + 1; j < scopes_.size(); ++j) {
				if (scope_func == scopes_[j].function_def()) {
					continue;
				}
				scope_func = scopes_[j].function_def();

				// 为upvalue分配变量
				find_var_info = &scopes_[j].AllocVar(var_name, var_info->flags);
				scope_func->AddClosureVar(find_var_info->var_idx, var_idx);
				var_idx = find_var_info->var_idx;
			}
		}
		break;
	}
	return find_var_info;
}

bool CodeGener::IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types) {
	for (ptrdiff_t i = scopes_.size() - 1; i >= 0; --i) {
		for (auto end_type : end_types) {
			if (scopes_[i].type() == end_type) {
				return false;
			}
		}
		
		for (auto type : types) {
			if (scopes_[i].type() == type) {
				return true;
			}
		}
	}
	return false;
}


const VarInfo& CodeGener::GetVarByExp(Exp* exp) {
	assert(exp->GetType() == ExpType::kIdentifier);

	auto& var_exp = exp->get<IdentifierExp>();
	auto var_info = FindVarIndexByName(var_exp.name);
	if (!var_info) {
		throw CodeGenerException("var not defined");
	}
	return *var_info;
}


void CodeGener::RegisterCppFunction(const std::string& func_name, CppFunction func) {
	auto& var_info = AllocVar(func_name, VarFlags::kConst);
	auto const_idx = AllocConst(Value(func));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_def_->byte_code().EmitConstLoad(const_idx);
	cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
}


Value CodeGener::Generate(BlockStat* block) {
	scopes_.clear();

	// 创建模块的函数定义
	cur_func_def_ = new FunctionDef("module", 0);
	cur_func_def_->SetModule();
	AllocConst(Value(cur_func_def_));

	EntryScope();

	RegisterCppFunction("println", [](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		for (size_t i = 0; i < par_count; i++) {
			auto val = stack.get(i);
			try {
				std::cout << val.ToString().string();
			}
			catch (const std::exception&)
			{
				std::cout << "unknown";
			}
		}
		printf("\n");
		return Value();
	});

	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kReturn);

	ExitScope();

	return Value(cur_func_def_);
	
}

void CodeGener::GenerateBlock(BlockStat* block, bool entry_scope, ScopeType type) {
	if (entry_scope) {
		EntryScope(nullptr, type);
	}
	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}
	if (entry_scope) {
		ExitScope();
	}
}

void CodeGener::GenerateStat(Stat* stat) {
	switch (stat->GetType()) {
	case StatType::kBlock: {
		GenerateBlock(&stat->get<BlockStat>());
		break;
	}
	case StatType::kExp: {
		auto exp = stat->get<ExpStat>().exp.get();
		// 抛弃纯表达式语句的最终结果
		if (exp) {
			GenerateExp(exp);
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);
		}
		break;
	}
	case StatType::kReturn: {
		GenerateReturnStat(&stat->get<ReturnStat>());
		break;
	}
	case StatType::kNewVar: {
		GenerateNewVarStat(&stat->get<NewVarStat>());
		break;
	}
	case StatType::kIf: {
		GenerateIfStat(&stat->get<IfStat>());
		break;
	}
	case StatType::kWhile: {
		GenerateWhileStat(&stat->get<WhileStat>());
		break;
	}
	case StatType::kContinue: {
		GenerateContinueStat(&stat->get<ContinueStat>());
		break;
	}
	case StatType::kBreak: {
		GenerateBreakStat(&stat->get<BreakStat>());
		break;
	}
	case StatType::kTry: {
		GenerateTryStat(&stat->get<TryStat>());
		break;
	}
	case StatType::kThrow: {
		GenerateThrowStat(&stat->get<ThrowStat>());
		break;
	}
	default:
		throw CodeGenerException("Unknown statement type");
	}
}

void CodeGener::GenerateReturnStat(ReturnStat* stat) {
	if (stat->exp.get()) {
		GenerateExp(stat->exp.get());
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	}
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyReturn);
	}
	else {
		cur_func_def_->byte_code().EmitReturn(cur_func_def_->type());
	}
}


void CodeGener::GenerateNewVarStat(NewVarStat* stat) {
	VarFlags flags = VarFlags::kNone;
	if (stat->keyword_type == TokenType::kKwConst) {
		flags = VarFlags::kConst;
	}

	auto& var_info = AllocVar(stat->var_name, flags);
	GenerateExp(stat->exp.get());
	// 弹出到变量中
	cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
}

// 2字节指的是基于当前指令的offset
void CodeGener::GenerateIfStat(IfStat* stat) {
	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 留给下一个else if/else修复
	auto if_pc = cur_func_def_->byte_code().Size();
	// 提前写入跳转的指令
	GenerateIfEq(stat->exp.get());

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

	std::vector<Pc> repair_end_pc_list;
	for (auto& else_if_stat : stat->else_if_stat_list) {
		repair_end_pc_list.push_back(cur_func_def_->byte_code().Size());
		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
		cur_func_def_->byte_code().EmitPcOffset(0);

		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_def_->byte_code().RepairPc(if_pc, cur_func_def_->byte_code().Size());

		// 表达式结果压栈
		GenerateExp(else_if_stat->exp.get());
		// 留给下一个else if/else修复
		if_pc = cur_func_def_->byte_code().Size();
		// 提前写入跳转的指令
		GenerateIfEq(else_if_stat->exp.get());

		GenerateBlock(else_if_stat->block.get());
	}

	if (stat->else_stat.get()) {
		repair_end_pc_list.push_back(cur_func_def_->byte_code().Size());
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_def_->byte_code().EmitPcOffset(0);

		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_def_->byte_code().RepairPc(if_pc, cur_func_def_->byte_code().Size());

		GenerateBlock(stat->else_stat->block.get());
	}
	else {
		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_def_->byte_code().RepairPc(if_pc, cur_func_def_->byte_code().Size());
	}

	// 至此if分支结构结束，修复所有退出分支结构的地址
	for (auto repair_end_pc : repair_end_pc_list) {
		cur_func_def_->byte_code().RepairPc(repair_end_pc, cur_func_def_->byte_code().Size());
	}
}

void CodeGener::GenerateWhileStat(WhileStat* stat) {
	auto save_cur_loop_repair_end_pc_list = cur_loop_repair_end_pc_list_;
	auto save_cur_loop_start_pc = cur_loop_start_pc_;

	std::vector<Pc> loop_repair_end_pc_list;
	cur_loop_repair_end_pc_list_ = &loop_repair_end_pc_list;

	// 记录重新循环的pc
	auto loop_start_pc = cur_func_def_->byte_code().Size();
	cur_loop_start_pc_ = loop_start_pc;

	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 等待修复
	loop_repair_end_pc_list.push_back(cur_func_def_->byte_code().Size());
	// 提前写入跳转的指令
	GenerateIfEq(stat->exp.get());

	GenerateBlock(stat->block.get(), true, ScopeType::kWhile);

	// 重新回去看是否需要循环
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	cur_func_def_->byte_code().EmitPcOffset(0);
	cur_func_def_->byte_code().RepairPc(cur_func_def_->byte_code().Size() - 3, loop_start_pc);

	for (auto repair_end_pc : loop_repair_end_pc_list) {
		// 修复跳出循环的指令的pc
		cur_func_def_->byte_code().RepairPc(repair_end_pc, cur_func_def_->byte_code().Size());
	}

	cur_loop_start_pc_ = save_cur_loop_start_pc;
	cur_loop_repair_end_pc_list_ = save_cur_loop_repair_end_pc_list;
}

void CodeGener::GenerateContinueStat(ContinueStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	// 跳回当前循环的起始pc
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyGoto);
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	}
	cur_func_def_->byte_code().EmitPcOffset(0);
	cur_func_def_->byte_code().RepairPc(cur_func_def_->byte_code().Size() - 3, cur_loop_start_pc_);
}

void CodeGener::GenerateBreakStat(BreakStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_loop_repair_end_pc_list_->push_back(cur_func_def_->byte_code().Size());
	// 无法提前得知结束pc，保存待修复pc，等待修复
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyGoto);
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	}
	cur_func_def_->byte_code().EmitPcOffset(0);
}

void CodeGener::GenerateTryStat(TryStat* stat) {
	auto has_finally = bool(stat->finally_stat);

	auto try_start_pc = cur_func_def_->byte_code().Size();

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kTryBegin);

	GenerateBlock(stat->block.get(), true, has_finally ? ScopeType::kTryFinally : ScopeType::kTry);

	auto try_end_pc = cur_func_def_->byte_code().Size();

	// 这里需要生成跳向finally的指令
	auto repair_end_pc = try_end_pc;
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	cur_func_def_->byte_code().EmitPcOffset(0);

	auto catch_start_pc = kInvalidPc;
	auto catch_end_pc = kInvalidPc;
	auto catch_err_var_idx = kVarInvaildIndex;

	if (stat->catch_stat) {
		catch_start_pc = cur_func_def_->byte_code().Size();
		EntryScope(nullptr, has_finally ? ScopeType::kCatchFinally : ScopeType::kCatch);

		// 加载error参数到变量
		catch_err_var_idx = AllocVar(stat->catch_stat->exp->name).var_idx;

		GenerateBlock(stat->catch_stat->block.get(), false);

		ExitScope();
		catch_end_pc = cur_func_def_->byte_code().Size();
	}
	else {
		catch_end_pc = try_end_pc;
	}

	// 修复pc
	cur_func_def_->byte_code().RepairPc(repair_end_pc, cur_func_def_->byte_code().Size());

	// finally是必定会执行的
	auto finally_start_pc = kInvalidPc;
	auto finally_end_pc = kInvalidPc;
	if (stat->finally_stat) {
		finally_start_pc = cur_func_def_->byte_code().Size();
		GenerateBlock(stat->finally_stat->block.get(), true, ScopeType::kFinally);
		finally_end_pc = cur_func_def_->byte_code().Size();
	}

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kTryEnd);

	if (!stat->catch_stat && !stat->finally_stat) {
		throw CodeGenerException("There cannot be a statement with only try.");
	}

	// 添加到异常表
	auto& exception_table = cur_func_def_->exception_table();
	auto exception_idx = exception_table.AddEntry({});
	auto& entry = exception_table.GetEntry(exception_idx);
	entry.try_start_pc = try_start_pc;
	entry.try_end_pc = try_end_pc;
	entry.catch_start_pc = catch_start_pc;
	entry.catch_end_pc = catch_end_pc;
	entry.catch_err_var_idx = catch_err_var_idx;
	entry.finally_start_pc = finally_start_pc;
	entry.finally_end_pc = finally_end_pc;
}

void CodeGener::GenerateThrowStat(ThrowStat* stat) {
	GenerateExp(stat->exp.get());
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kThrow);
}



void CodeGener::GenerateExp(Exp* exp) {
	switch (exp->GetType()) {
	case ExpType::kFunctionDecl:
		GenerateFunctionDeclExp(&exp->get<FuncDeclExp>());
		break;
	case ExpType::kUndefined:
	case ExpType::kNull:
	case ExpType::kBool:
	case ExpType::kNumber:
	case ExpType::kString:
	case ExpType::kArrayLiteralExp:
	case ExpType::kObjectLiteralExp: {
		auto const_idx = AllocConst(MakeValue(exp));
		cur_func_def_->byte_code().EmitConstLoad(const_idx);
		break;
	}
	case ExpType::kIdentifier: {
		// 如果是标识符的话，找下ClassDefTable
		auto& ident_exp = exp->get<IdentifierExp>();

		auto class_def = runtime_->class_def_table().find(ident_exp.name);
		// 先不考虑js里定义的类
		if (class_def) {
			auto const_idx = AllocConst(Value(class_def));
			cur_func_def_->byte_code().EmitConstLoad(const_idx);
		}
		else {
			// throw CodeGenerException("Undefined class.");
			// 是取变量值的话，查找到对应的变量编号，将其入栈
			const auto& var_info = GetVarByExp(exp);
			cur_func_def_->byte_code().EmitVarLoad(var_info.var_idx);	// 从变量中获取
		}
		break;
	}
	case ExpType::kThis: {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetThis);
		break;
	}
	case ExpType::kIndexedExp: {
		auto& idx_exp = exp->get<IndexedExp>();

		// 被访问的表达式，应该是一个数组对象，入栈这个表达式
		GenerateExp(idx_exp.exp.get());

		// 判断下是否调用函数，是则dump
		if (idx_exp.is_method_call) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kDump);
		}

		// 用于访问的下标的表达式，入栈这个表达式
		//if (idx_exp->index_exp->GetType() == ExpType::kNumber) {
		//	// 如果是number表达式，可以提前预处理为StringValue

		//}
		//else {
			GenerateExp(idx_exp.index_exp.get());
		//}

		// 生成索引访问的指令
		cur_func_def_->byte_code().EmitIndexedLoad();

		break;
	}
	case ExpType::kDotExp: {
		auto& dot_exp = exp->get<MemberExp>();

		// 成员访问表达式
		auto prop_exp = dot_exp.prop_exp.get();

		//if (prop_exp->GetType() != ExpType::kIdentifier) {
		//	throw CodeGenerException("Incorrect right value for attribute access.");
		//}

		// 左值表达式入栈
		GenerateExp(dot_exp.exp.get());

		// 判断下是否调用函数，是则dump
		if (dot_exp.is_method_call) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kDump);
		}

		// 访问对象成员
		auto const_idx = AllocConst(MakeValue(prop_exp));
		cur_func_def_->byte_code().EmitConstLoad(const_idx);

		cur_func_def_->byte_code().EmitPropertyLoad();

		return;
	}
	case ExpType::kUnaryOp: {
		auto& unary_op_exp = exp->get<UnaryOpExp>();
		// 表达式的值入栈
		GenerateExp(unary_op_exp.operand.get());

		// 生成运算指令
		switch (unary_op_exp.oper) {
		case TokenType::kOpSub:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kNeg);
			break;
		case TokenType::kKwAwait:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kAwait);
			break;
		case TokenType::kOpPrefixInc:
		case TokenType::kOpSuffixInc:
		default:
			throw CodeGenerException("Unrecognized unary operator");
		}
		break;
	}
	case ExpType::kBinaryOp: {
		auto& bina_exp = exp->get<BinaryOpExp>();

		switch (bina_exp.oper) {
		case TokenType::kOpAssign: {
			// 赋值表达式
			
			// 右值表达式先入栈
			GenerateExp(bina_exp.right_exp.get());

			auto lvalue_exp = bina_exp.left_exp.get();
			if (lvalue_exp->value_category != ExpValueCategory::kLeftValue) {
				throw CodeGenerException("The left side of the assignment operator must be an lvalue.");
			}

			// 再处理左值表达式
			switch (lvalue_exp->GetType())
			{
			// 为左值赋值
			case ExpType::kIdentifier: {
				const auto& var_info = GetVarByExp(lvalue_exp);
				if ((var_info.flags & VarFlags::kConst) == VarFlags::kConst) {
					throw CodeGenerException("Cannot change const var.");
				}

				cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
				break;
			}
			case ExpType::kIndexedExp: {
				auto& indexed_exp = lvalue_exp->get<IndexedExp>();

				// obj["a"]["b"] = 100;

				// 入栈的是obj["a"]
				GenerateExp(indexed_exp.exp.get());

				// 入栈["b"]
				GenerateExp(indexed_exp.index_exp.get());

				cur_func_def_->byte_code().EmitIndexedStore();

				break;
			}
			case ExpType::kDotExp: {
				auto& dot_exp = lvalue_exp->get<MemberExp>();

				// 设置对象的属性
				// 如：obj.a.b = 100;
				// 先入栈obj.a这个对象
				GenerateExp(dot_exp.exp.get());

				auto& prop_exp = dot_exp.prop_exp->get<IdentifierExp>();
				auto const_idx = AllocConst(MakeValue(&prop_exp));
				cur_func_def_->byte_code().EmitConstLoad(const_idx);

				// 设置栈顶对象的成员
				cur_func_def_->byte_code().EmitPropertyStore();
				
				break;
			}
			default:
				throw CodeGenerException("Lvalue expression type error.");
			}

			// 完成后，需要再将左值再次入栈
			GenerateExp(lvalue_exp);
			return;
		}
		}

		// 其他二元运算
		
		// 左右表达式的值入栈
		GenerateExp(bina_exp.left_exp.get());
		GenerateExp(bina_exp.right_exp.get());

		// 生成运算指令
		switch (bina_exp.oper) {
		case TokenType::kOpAdd:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kAdd);
			break;
		case TokenType::kOpSub:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kSub);
			break;
		case TokenType::kOpMul:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kMul);
			break;
		case TokenType::kOpDiv:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kDiv);
			break;
		case TokenType::kOpNe:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kNe);
			break;
		case TokenType::kOpEq:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kEq);
			break;
		case TokenType::kOpLt:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kLt);
			break;
		case TokenType::kOpLe:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kLe);
			break;
		case TokenType::kOpGt:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGt);
			break;
		case TokenType::kOpGe:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGe);
			break;
		default:
			throw CodeGenerException("Unrecognized binary operator");
		}
		break;
	}
	case ExpType::kNew: {
		auto& new_exp = exp->get<NewExp>();
		GenerateParList(new_exp.par_list);
		
		GenerateExp(new_exp.callee.get());
		
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kNew);
		break;
	}
	case ExpType::kFunctionCall: {
		auto& func_call_exp = exp->get<FunctionCallExp>();

		//if (func_call_exp->par_list.size() < const_table_[]->function_def()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		GenerateParList(func_call_exp.par_list);
		GenerateExp(func_call_exp.func_obj.get());

		// 将this置于栈顶
		if (func_call_exp.func_obj->GetType() == ExpType::kDotExp) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kSwap);
		}
		else if (func_call_exp.func_obj->GetType() == ExpType::kIndexedExp) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kSwap);
		}
		else {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
		}

		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFunctionCall);

		// auto var_idx = GetVarByExp(func_call_exp->func_obj.get());
		// cur_func_->byte_code().EmitPcOffset(var_idx);

		break;
	}
	case ExpType::kYield: {
		GenerateExp(exp->get<YieldExp>().exp.get());

		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kYield);
		break;
	}
	default:
		throw CodeGenerException("Unrecognized exp");
		break;
	}
}

void CodeGener::GenerateFunctionDeclExp(FuncDeclExp* exp) {
	auto const_idx = AllocConst(Value(new FunctionDef(exp->func_name, exp->par_list.size())));
	auto& func_def = GetConstValueByIndex(const_idx).function_def();
	if (exp->func_type == FunctionType::kGenerator) {
		func_def.SetGenerator();
	}
	else if (exp->func_type == FunctionType::kAsync) {
		func_def.SetAsync();
	}
	cur_func_def_->byte_code().EmitConstLoad(const_idx);

	if (!exp->func_name.empty()) {
		// 非匿名函数分配变量来装，这里其实有个没考虑的地方
		// 如果外层还有一层赋值，那么该函数的名字应该只在函数内作用域有效
		auto& var_info = AllocVar(exp->func_name, VarFlags::kConst);
		cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);

		// 栈顶的被赋值消耗了，再push一个
		cur_func_def_->byte_code().EmitConstLoad(const_idx);
	}

	// 保存环境，以生成新指令流
	auto savefunc = cur_func_def_;

	// 切换环境
	EntryScope(&func_def, ScopeType::kFunction);
	cur_func_def_ = &func_def;

	// 参数正序分配
	for (size_t i = 0; i < cur_func_def_->par_count(); ++i) {
		AllocVar(exp->par_list[i]);
	}

	auto block = exp->block.get();
	for (size_t i = 0; i < block->stat_list.size(); i++) {
		auto& stat = block->stat_list[i];
		GenerateStat(stat.get());
		if (i == block->stat_list.size() - 1) {
			if (stat->GetType() != StatType::kReturn) {
				// 补全末尾的return
				cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
				cur_func_def_->byte_code().EmitReturn(cur_func_def_->type());
			}
		}
	}

	// 恢复环境
	ExitScope();
	cur_func_def_ = savefunc;
}

void CodeGener::GenerateIfEq(Exp* exp) {
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kIfEq);
	cur_func_def_->byte_code().EmitPcOffset(0);
}

Value CodeGener::MakeValue(Exp* exp) {
	switch (exp->GetType()) {
	case ExpType::kUndefined:{
		return Value();
	}
	case ExpType::kNull: {
		return Value(nullptr);
	}
	case ExpType::kBool: {
		return Value(exp->get<BoolExp>().value);
	}
	case ExpType::kNumber: {
		return Value(exp->get<NumberExp>().value);
	}
	case ExpType::kString: {
		return Value(exp->get<StringExp>().value);
	}
	case ExpType::kIdentifier: {
		return Value(exp->get<IdentifierExp>().name);
	}
	// 无需GC回收，此处分配的对象不会引用Context分配的对象，因此不存在循环引用
	// 应该是只读
	case ExpType::kArrayLiteralExp: {
		ArrayObject* arr_obj = new ArrayObject(nullptr);
		double i = 0;
		for (auto& exp : exp->get<ArrayLiteralExp>().arr_litera) {
			// arr_obj->mutale_values().emplace_back(MakeValue(exp.get()));
			auto const_idx = AllocConst(Value(i++)/*.ToString()*/);
			arr_obj->SetProperty(nullptr, GetConstValueByIndex(const_idx), MakeValue(exp.get()));
		}
		return Value(arr_obj);
	}
	case ExpType::kObjectLiteralExp: {
		Object* obj = new Object(nullptr);
		for (auto& exp : exp->get<ObjectLiteralExp>().obj_litera) {
			auto const_idx = AllocConst(Value(exp.first));
			obj->SetProperty(nullptr, GetConstValueByIndex(const_idx), MakeValue(exp.second.get()));
		}
		return Value(obj);
	}
	default:
		throw CodeGenerException("Unable to generate expression for value");
	}
}

void CodeGener::GenerateParList(const std::vector<std::unique_ptr<Exp>>& par_list) {
	// 参数正序入栈
	for (size_t i = 0; i < par_list.size(); ++i) {
		GenerateExp(par_list[i].get());
	}

	auto const_idx = AllocConst(Value(par_list.size()));
	cur_func_def_->byte_code().EmitConstLoad(const_idx);
}

} // namespace mjs
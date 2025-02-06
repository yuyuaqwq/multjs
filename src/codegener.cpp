#include "codegener.h"

#include <iostream>

#include <mjs/runtime.h>
#include <mjs/arr_obj.h>

namespace mjs {

CodeGener::CodeGener(Runtime* runtime)
	: runtime_(runtime) {}

void CodeGener::EntryScope(FunctionDefObject* sub_func) {
	scopes_.emplace_back(sub_func);
}

void CodeGener::ExitScope() {
	scopes_.pop_back();
}


ConstIndex CodeGener::AllocConst(Value&& value) {
	return runtime_->const_pool().New(std::move(value));
}

const Value& CodeGener::FindConstValueByIndex(ConstIndex idx) {
	return runtime_->const_pool().Get(idx);
}


VarIndex CodeGener::AllocVar(const std::string& var_name) {
	return scopes_.back().AllocVar(var_name);
}

std::optional<VarIndex> CodeGener::FindVarIndexByName(const std::string& var_name) {
	std::optional<VarIndex> find_var_idx = std::nullopt;
	// 就近找变量
	for (ptrdiff_t i = scopes_.size() - 1; i >= 0; --i) {
		auto var_idx_opt = scopes_[i].FindVar(var_name);
		if (!var_idx_opt) {
			// 当前作用域找不到变量，向上层作用域找
			continue;
		}
		if (scopes_[i].func() == cur_func_) {
			find_var_idx = *var_idx_opt;
		}
		else {
			// 在上层函数作用域找到了，构建upvalue捕获链
			auto scope_func = scopes_[i].func();
			scope_func->closure_var_defs_.emplace(
				*var_idx_opt,
				FunctionDefObject::ClosureVarDef{
					.arr_idx = int32_t(scope_func->closure_var_defs_.size()),
				}
			);

			for (size_t j = i + 1; j < scopes_.size(); ++j) {
				if (scope_func == scopes_[j].func()) {
					continue;
				}
				scope_func = scopes_[j].func();

				// 为upvalue分配变量
				find_var_idx = scopes_[j].AllocVar(var_name);
				scope_func->closure_var_defs_.emplace(
					*find_var_idx,
					FunctionDefObject::ClosureVarDef{
						.arr_idx = int32_t(scope_func->closure_var_defs_.size()),
						.parent_var_idx = *var_idx_opt
					}
				);

				*var_idx_opt = *find_var_idx;
			}
		}
		break;
	}
	return find_var_idx;
}

VarIndex CodeGener::GetVarByExp(Exp* exp) {
	assert(exp->GetType() == ExpType::kIdentifier);
	auto var_exp = static_cast<IdentifierExp*>(exp);
	auto var_idx = FindVarIndexByName(var_exp->name);
	if (!var_idx) {
		throw CodeGenerException("var not defined");
	}
	return *var_idx;
}


void CodeGener::RegistryFunctionBridge(const std::string& func_name, FunctionBridgeObject func) {
	auto var_idx = AllocVar(func_name);
	auto const_idx = AllocConst(Value(func));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_->byte_code.EmitConstLoad(const_idx);
	cur_func_->byte_code.EmitVarStore(var_idx);
}


Value CodeGener::Generate(BlockStat* block) {
	scopes_.clear();

	// 创建顶层函数(模块)
	auto const_idx = AllocConst(Value(new FunctionDefObject(0)));
	cur_func_ = runtime_->const_pool().Get(const_idx).function_def();

	scopes_.emplace_back(cur_func_);

	RegistryFunctionBridge("println",
		[](uint32_t par_count, StackFrame* stack) -> Value {
			for (size_t i = 0; i < par_count; i++) {
				auto val = stack->Get(i);
				std::cout << val.ToString().string_u8();
			}
			printf("\n");
			return Value();
		});

	for (auto& stat : block->stat_list) {
		GenerateStat(stat.get());
	}

	// cur_func_->byte_code.EmitOpcode(OpcodeType::kStop);
	return Value(cur_func_);
	
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
			cur_func_->byte_code.EmitOpcode(OpcodeType::kPop);
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
	auto const_idx = AllocConst(Value(new FunctionDefObject(stat->par_list.size())));
	cur_func_->byte_code.EmitConstLoad(const_idx);

	auto func_def = runtime_->const_pool().Get(const_idx).function_def();

	auto var_idx = AllocVar(stat->func_name);
	cur_func_->byte_code.EmitVarStore(var_idx);

	// 保存环境，以生成新指令流
	auto savefunc = cur_func_;

	// 切换环境
	EntryScope(func_def);
	cur_func_ = func_def;

	// 参数正序分配
	for (size_t i = 0; i < cur_func_->par_count; ++i) {
		AllocVar(stat->par_list[i]);
	}

	auto block = stat->block.get();
	for (size_t i = 0; i < block->stat_list.size(); i++) {
		auto& stat = block->stat_list[i];
		GenerateStat(stat.get());
		if (i == block->stat_list.size() - 1) {
			if (stat->GetType() != StatType::kReturn) {
				// 补全末尾的return
				cur_func_->byte_code.EmitConstLoad(AllocConst(Value()));
				cur_func_->byte_code.EmitOpcode(OpcodeType::kReturn);
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
		cur_func_->byte_code.EmitConstLoad(AllocConst(Value()));
	}
	cur_func_->byte_code.EmitOpcode(OpcodeType::kReturn);
}

void CodeGener::GenerateNewVarStat(NewVarStat* stat) {
	auto var_idx = AllocVar(stat->var_name);
	GenerateExp(stat->exp.get());
	// 弹出到变量中
	cur_func_->byte_code.EmitVarStore(var_idx);
}

// 2字节指的是基于当前指令的offset
void CodeGener::GenerateIfStat(IfStat* stat) {
	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 留给下一个else if/else修复
	auto if_pc = cur_func_->byte_code.Size();
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
		repair_end_pc_list.push_back(cur_func_->byte_code.Size());
		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_->byte_code.EmitOpcode(OpcodeType::kGoto);
		cur_func_->byte_code.EmitPcOffset(0);

		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_->byte_code.RepairPc(if_pc, cur_func_->byte_code.Size());

		// 表达式结果压栈
		GenerateExp(else_if_stat->exp.get());
		// 留给下一个else if/else修复
		if_pc = cur_func_->byte_code.Size();
		// 提前写入跳转的指令
		GenerateIfEq(else_if_stat->exp.get());

		GenerateBlock(else_if_stat->block.get());
	}

	if (stat->else_stat.get()) {
		repair_end_pc_list.push_back(cur_func_->byte_code.Size());
		cur_func_->byte_code.EmitOpcode(OpcodeType::kGoto);		// 提前写入上一分支退出if分支结构的jmp跳转
		cur_func_->byte_code.EmitPcOffset(0);

		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_->byte_code.RepairPc(if_pc, cur_func_->byte_code.Size());

		GenerateBlock(stat->else_stat->block.get());
	}
	else {
		// 修复条件为false时，跳转到if/else if块之后的地址
		cur_func_->byte_code.RepairPc(if_pc, cur_func_->byte_code.Size());
	}

	// 至此if分支结构结束，修复所有退出分支结构的地址
	for (auto repair_pnd_pc : repair_end_pc_list) {
		cur_func_->byte_code.RepairPc(repair_pnd_pc, cur_func_->byte_code.Size());
	}
}

void CodeGener::GenerateWhileStat(WhileStat* stat) {
	auto save_cur_loop_repair_end_pc_list = cur_loop_repair_end_pc_list_;
	auto save_cur_loop_start_pc = cur_loop_start_pc_;

	std::vector<Pc> loop_repair_end_pc_list;
	cur_loop_repair_end_pc_list_ = &loop_repair_end_pc_list;

	// 记录重新循环的pc
	Pc loop_start_pc = cur_func_->byte_code.Size();
	cur_loop_start_pc_ = loop_start_pc;

	// 表达式结果压栈
	GenerateExp(stat->exp.get());

	// 等待修复
	loop_repair_end_pc_list.push_back(cur_func_->byte_code.Size());
	// 提前写入跳转的指令
	GenerateIfEq(stat->exp.get());

	GenerateBlock(stat->block.get());

	// 重新回去看是否需要循环
	cur_func_->byte_code.EmitOpcode(OpcodeType::kGoto);
	cur_func_->byte_code.EmitPcOffset(0);
	cur_func_->byte_code.RepairPc(cur_func_->byte_code.Size() - 3, loop_start_pc);

	for (auto repair_end_pc : loop_repair_end_pc_list) {
		// 修复跳出循环的指令的pc
		cur_func_->byte_code.RepairPc(repair_end_pc, cur_func_->byte_code.Size());
	}

	cur_loop_start_pc_ = save_cur_loop_start_pc;
	cur_loop_repair_end_pc_list_ = save_cur_loop_repair_end_pc_list;
}

void CodeGener::GenerateContinueStat(ContinueStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	// 跳回当前循环的起始pc
	cur_func_->byte_code.EmitOpcode(OpcodeType::kGoto);
	cur_func_->byte_code.EmitPcOffset(0);
	cur_func_->byte_code.RepairPc(cur_func_->byte_code.Size() - 3, cur_loop_start_pc_);
}

void CodeGener::GenerateBreakStat(BreakStat* stat) {
	if (cur_loop_repair_end_pc_list_ == nullptr) {
		throw CodeGenerException("Cannot use break in acyclic scope");
	}
	cur_loop_repair_end_pc_list_->push_back(cur_func_->byte_code.Size());
	// 无法提前得知结束pc，保存待修复pc，等待修复
	cur_func_->byte_code.EmitOpcode(OpcodeType::kGoto);
	cur_func_->byte_code.EmitPcOffset(0);
}


void CodeGener::GenerateExp(Exp* exp) {
	switch (exp->GetType()) {
	case ExpType::kNull:
	case ExpType::kBool:
	case ExpType::kNumber:
	case ExpType::kString:
	case ExpType::kArrayLiteralExp:
	case ExpType::kObjectLiteralExp: {
		auto const_idx = AllocConst(MakeValue(exp));
		cur_func_->byte_code.EmitConstLoad(const_idx);
		break;
	}
	case ExpType::kIdentifier: {
		// 是取变量值的话，查找到对应的变量编号，将其入栈
		auto var_idx = GetVarByExp(exp);
		cur_func_->byte_code.EmitVarLoad(var_idx);	// 从变量中获取
		break;
	}
	case ExpType::kIndexedExp: {
		auto idx_exp = static_cast<IndexedExp*>(exp);

		// 被访问的表达式，应该是一个数组对象，入栈这个表达式
		GenerateExp(idx_exp->exp.get());

		// 用于访问的下标的表达式，入栈这个表达式
		//if (idx_exp->index_exp->GetType() == ExpType::kNumber) {
		//	// 如果是number表达式，可以提前预处理为StringValue

		//}
		//else {
			GenerateExp(idx_exp->index_exp.get());
		//}

		// 生成索引访问的指令
		cur_func_->byte_code.EmitIndexedLoad();

		break;
	}
	case ExpType::kDotExp: {
		auto dot_exp = static_cast<DotExp*>(exp);

		// 成员访问表达式
		auto prop_exp = dot_exp->prop_exp.get();

		// 可能是调用函数
		if (prop_exp->GetType() == ExpType::kFunctionCall) {
			auto func_call = static_cast<FunctionCallExp*>(prop_exp);
			GenerateFunctionCallPar(func_call);

			// 左值表达式入栈
			GenerateExp(dot_exp->exp.get());

			auto const_idx = AllocConst(MakeValue(func_call->func_name.get()));
			cur_func_->byte_code.EmitConstLoad(const_idx);

			cur_func_->byte_code.EmitPropertyCall();
		}
		else if (prop_exp->GetType() == ExpType::kIdentifier) {
			// 左值表达式入栈
			GenerateExp(dot_exp->exp.get());

			// 访问对象成员
			auto const_idx = AllocConst(MakeValue(prop_exp));
			cur_func_->byte_code.EmitConstLoad(const_idx);

			cur_func_->byte_code.EmitPropertyLoad();
		}
		else {
			throw CodeGenerException("Incorrect right value for attribute access.");
		}
		return;
	}
	case ExpType::kUnaryOp: {
		auto unary_op_exp = static_cast<UnaryOpExp*>(exp);
		// 表达式的值入栈
		GenerateExp(unary_op_exp->operand.get());

		// 生成运算指令
		switch (unary_op_exp->oper) {
		case TokenType::kOpSub:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kNeg);
			break;
		case TokenType::kOpPrefixInc: {

			break;
		}
		case TokenType::kOpSuffixInc: {

			break;
		}
		default:
			throw CodeGenerException("Unrecognized unary operator");
		}
		break;
	}
	case ExpType::kBinaryOp: {
		auto bina_exp = static_cast<BinaryOpExp*>(exp);

		switch (bina_exp->oper) {
		case TokenType::kOpAssign: {
			// 赋值表达式
			
			// 右值表达式先入栈
			GenerateExp(bina_exp->right_exp.get());

			auto lvalue_exp = bina_exp->left_exp.get();
			if (lvalue_exp->value_category != ExpValueCategory::kLeftValue) {
				throw CodeGenerException("The left side of the assignment operator must be an lvalue.");
			}

			// 再处理左值表达式
			switch (lvalue_exp->GetType())
			{
			// 为左值赋值
			case ExpType::kIdentifier: {
				auto var_idx = GetVarByExp(exp);
				cur_func_->byte_code.EmitVarStore(var_idx);	// 从变量中获取
				break;
			}
			case ExpType::kIndexedExp: {
				auto indexed_exp = static_cast<IndexedExp*>(lvalue_exp);

				// obj["a"]["b"] = 100;

				// 入栈的是obj["a"]
				GenerateExp(indexed_exp->exp.get());

				// 入栈["b"]
				GenerateExp(indexed_exp->index_exp.get());

				cur_func_->byte_code.EmitIndexedStore();

				break;
			}
			case ExpType::kDotExp: {
				auto dot_exp = static_cast<DotExp*>(lvalue_exp);

				// 设置对象的属性
				// 如：obj.a.b = 100;
				// 先入栈obj.a这个对象
				GenerateExp(dot_exp->exp.get());

				auto prop_exp = static_cast<IdentifierExp*>(dot_exp->prop_exp.get());
				auto const_idx = AllocConst(MakeValue(prop_exp));
				cur_func_->byte_code.EmitConstLoad(const_idx);

				// 设置栈顶对象的成员
				cur_func_->byte_code.EmitPropertyStore();
				
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
		GenerateExp(bina_exp->left_exp.get());
		GenerateExp(bina_exp->right_exp.get());

		// 生成运算指令
		switch (bina_exp->oper) {
		case TokenType::kOpAdd:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kAdd);
			break;
		case TokenType::kOpSub:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kSub);
			break;
		case TokenType::kOpMul:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kMul);
			break;
		case TokenType::kOpDiv:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kDiv);
			break;
		case TokenType::kOpNe:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kNe);
			break;
		case TokenType::kOpEq:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kEq);
			break;
		case TokenType::kOpLt:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kLt);
			break;
		case TokenType::kOpLe:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kLe);
			break;
		case TokenType::kOpGt:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kGt);
			break;
		case TokenType::kOpGe:
			cur_func_->byte_code.EmitOpcode(OpcodeType::kGe);
			break;
		default:
			throw CodeGenerException("Unrecognized binary operator");
		}
		break;
	}
	case ExpType::kFunctionCall: {
		auto func_call_exp = static_cast<FunctionCallExp*>(exp);

		auto var_idx = GetVarByExp(func_call_exp->func_name.get());

		//if (func_call_exp->par_list.size() < const_table_[]->function_def()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		GenerateFunctionCallPar(func_call_exp);
		cur_func_->byte_code.EmitOpcode(OpcodeType::kFunctionCall);
		cur_func_->byte_code.EmitPcOffset(var_idx);

		break;
	}
	default:
		throw CodeGenerException("Unrecognized exp");
		break;
	}
}

void CodeGener::GenerateIfEq(Exp* exp) {
	cur_func_->byte_code.EmitOpcode(OpcodeType::kIfEq);
	cur_func_->byte_code.EmitPcOffset(0);
}

Value CodeGener::MakeValue(Exp* exp) {
	switch (exp->GetType()) {
	case ExpType::kNull: {
		return Value(nullptr);
	}
	case ExpType::kBool: {
		auto bool_exp = static_cast<BoolExp*>(exp);
		return Value(bool_exp->value);
	}
	case ExpType::kNumber: {
		auto num_exp = static_cast<NumberExp*>(exp);
		return Value(num_exp->value);
	}
	case ExpType::kString: {
		auto str_exp = static_cast<StringExp*>(exp);
		return Value(str_exp->value);
	}
	case ExpType::kIdentifier: {
		auto ident_exp = static_cast<IdentifierExp*>(exp);
		return Value(ident_exp->name);
	}
	case ExpType::kArrayLiteralExp: {
		auto arr_exp = static_cast<ArrayLiteralExp*>(exp);
		ArrayObject* arr_obj = new ArrayObject();
		double i = 0;
		for (auto& exp : arr_exp->arr_litera) {
			// arr_obj->mutale_values().emplace_back(MakeValue(exp.get()));
			auto const_idx = AllocConst(Value(i++).ToString());
			arr_obj->SetProperty(FindConstValueByIndex(const_idx), MakeValue(exp.get()));
		}
		return Value(arr_obj);
	}
	case ExpType::kObjectLiteralExp: {
		auto obj_exp = static_cast<ObjectLiteralExp*>(exp);
		Object* obj = new Object();
		for (auto& exp : obj_exp->obj_litera) {
			auto const_idx = AllocConst(Value(exp.first));
			obj->SetProperty(FindConstValueByIndex(const_idx), MakeValue(exp.second.get()));
		}
		return Value(obj);
	}
	default:
		throw CodeGenerException("Unable to generate expression for value");
	}
}

void CodeGener::GenerateFunctionCallPar(FunctionCallExp* func_call_exp) {
	// 参数正序入栈
	for (size_t i = 0; i < func_call_exp->par_list.size(); ++i) {
		GenerateExp(func_call_exp->par_list[i].get());
	}

	auto const_idx = AllocConst(Value(func_call_exp->par_list.size()));
	cur_func_->byte_code.EmitConstLoad(const_idx);
}

} // namespace mjs
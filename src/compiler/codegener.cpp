#include "codegener.h"

#include <iostream>

#include <mjs/error.h>
#include <mjs/context.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>

namespace mjs {
namespace compiler {

CodeGener::CodeGener(Context* context, Parser* parser)
	: context_(context)
	, parser_(parser){}

void CodeGener::AddCppFunction(const std::string& func_name, CppFunction func) {
	auto& var_info = AllocVar(func_name, VarFlags::kConst);
	auto const_idx = AllocConst(Value(func));

	// 生成将函数放到变量表中的代码
	// 交给虚拟机执行时去加载，虚拟机发现加载的常量是函数体，就会将函数原型赋给局部变量
	cur_func_def_->byte_code().EmitConstLoad(const_idx);
	cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);
}

//void CodeGener::AddCppModule() {
//	auto decl = ImportDeclaration(0, 0, std::move(source), std::move(name));
//	GenerateImportDeclaration(&decl);
//}

Value CodeGener::Generate(std::string&& module_name) {
	scopes_.clear();

	// 创建模块的函数定义
	cur_func_def_ = new ModuleDef(&context_->runtime(), std::move(module_name), 0);
	cur_func_def_->set_is_module();
	AllocConst(Value(cur_func_def_));

	EntryScope();

	for (auto& decl : parser_->import_declarations()) {
		GenerateStatement(decl.get());
	}

	for (auto& stat : parser_->src_statements()) {
		GenerateStatement(stat.get());
	}

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kReturn);

	ExitScope();

	return Value(static_cast<ModuleDef*>(cur_func_def_));

}



void CodeGener::GenerateExpression(Expression* exp) {
	switch (exp->type()) {
	case ExpressionType::kUndefined:
	case ExpressionType::kNull:
	case ExpressionType::kBoolean:
	case ExpressionType::kInteger:
	case ExpressionType::kFloat:
	case ExpressionType::kString: {
		auto const_idx = AllocConst(MakeConstValue(exp));
		cur_func_def_->byte_code().EmitConstLoad(const_idx);
		break;
	}
	case ExpressionType::kArrayExpression: {
		GeneratorArrayExpression(&exp->as<ArrayExpression>());
		break;
	}
	case ExpressionType::kObjectExpression: {
		GeneratorObjectExpression(&exp->as<ObjectExpression>());
		break;
	}
	case ExpressionType::kIdentifier: {
		// 如果是标识符的话，找下ClassDefTable
		auto& ident_exp = exp->as<Identifier>();

		//auto class_def = runtime_->class_def_table().find(ident_exp.name());
		//// 先不考虑js里定义的类
		//if (class_def) {
		//	auto const_idx = AllocConst(Value(class_def));
		//	cur_func_def_->byte_code().EmitConstLoad(const_idx);
		//}
		//else {
			// throw CodeGenerException("Undefined class.");
			// 尝试查找到对应的变量索引
			const auto* var_info = GetVarByExpression(exp);
			if (var_info) {
				// 从变量中获取
				cur_func_def_->byte_code().EmitVarLoad(var_info->var_idx);
			}
			else {
				// 尝试从全局对象获取
				auto const_idx = AllocConst(Value(String::New(ident_exp.name())));
				cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetGlobal);
				cur_func_def_->byte_code().EmitI32(const_idx);
			}
		// }
		break;
	}
	case ExpressionType::kThisExpression: {
		cur_func_def_->set_has_this(true);
		if (IsInTypeScope({ ScopeType::kFunction }, { ScopeType::kArrowFunction })) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetThis);
		}
		else {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetOuterThis);
		}
		break;
	}
	case ExpressionType::kMemberExpression: {
		auto& mem_exp = exp->as<MemberExpression>();

		// 被访问的表达式，入栈这个表达式
		GenerateExpression(mem_exp.object().get());
		// 判断下是否调用函数，是则dump
		if (mem_exp.is_method_call()) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kDump);
		}

		if (mem_exp.computed()) {
			// 用于访问的下标的表达式，入栈这个表达式
			GenerateExpression(mem_exp.property().get());

			// 生成索引访问的指令
			cur_func_def_->byte_code().EmitIndexedLoad();
		}
		else {
			// 成员访问表达式
			auto& prop_exp = mem_exp.property()->as<Identifier>();

			//if (prop_exp->GetType() != ExpType::kIdentifier) {
			//	throw CodeGenerException("Incorrect right value for attribute access.");
			//}

			// 访问对象成员
			auto const_idx = AllocConst(Value(String::New(prop_exp.name())));
			cur_func_def_->byte_code().EmitPropertyLoad(const_idx);
		}
		break;
	}
	case ExpressionType::kFunctionExpression:
		GenerateFunctionExpression(&exp->as<FunctionExpression>());
		break;
	case ExpressionType::kArrowFunctionExpression: {
		GenerateArrowFunctionExpression(&exp->as<ArrowFunctionExpression>());
		break;
	}
	case ExpressionType::kUnaryExpression: {
		auto& unary_exp = exp->as<UnaryExpression>();

		// 表达式的值入栈
		GenerateExpression(unary_exp.argument().get());

		// 生成运算指令
		switch (unary_exp.op()) {
		case TokenType::kOpSub:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kNeg);
			break;
		case TokenType::kKwAwait:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kAwait);
			break;
		case TokenType::kOpPrefixInc:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kInc);
			GenerateLValueStore(unary_exp.argument().get());
			break;
		case TokenType::kOpSuffixInc:
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kDump);
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kInc);
			GenerateLValueStore(unary_exp.argument().get());
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);
			break;
		default:
			throw SyntaxError("Unrecognized unary operator.");
		}
		break;
	}
	case ExpressionType::kAssignmentExpression: {
		// 赋值表达式
		auto& assign_exp = exp->as<AssignmentExpression>();

		// 右值表达式先入栈
		GenerateExpression(assign_exp.right().get());

		auto lvalue_exp = assign_exp.left().get();
		GenerateLValueStore(lvalue_exp);

		return;
	}
	case ExpressionType::kBinaryExpression: {
		auto& bina_exp = exp->as<BinaryExpression>();

		// 其他二元运算

		// 左右表达式的值入栈
		GenerateExpression(bina_exp.left().get());
		GenerateExpression(bina_exp.right().get());

		// 生成运算指令
		switch (bina_exp.op()) {
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
		case TokenType::kSepComma:
			break;
		default:
			throw SyntaxError("Unrecognized binary operator.");
		}
		break;
	}
	case ExpressionType::kNewExpression: {
		auto& new_exp = exp->as<NewExpression>();
		GenerateParamList(new_exp.arguments());

		//if (new_exp.callee()->is(ExpressionType::kIdentifier)) {
		//	auto class_def = runtime_->class_def_table().find(new_exp.callee()->as<Identifier>().name());
		//	// todo:先不考虑js里定义的类
		//	if (class_def) {
		//		auto const_idx = AllocConst(Value(ValueType::kNewConstructor, class_def));
		//		cur_func_def_->byte_code().EmitConstLoad(const_idx);
		//	}
		//}
		//else {
			GenerateExpression(new_exp.callee().get());
		//}

		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kNew);
		break;
	}
	case ExpressionType::kCallExpression: {
		auto& call_exp = exp->as<CallExpression>();

		//if (func_call_exp->par_list.size() < const_table_[]->function_def()->par_count) {
		//	throw CodeGenerException("Wrong number of parameters passed during function call");
		//}

		GenerateParamList(call_exp.arguments());
		GenerateExpression(call_exp.callee().get());

		// 将this置于栈顶
		if (call_exp.callee()->is(ExpressionType::kMemberExpression)) {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kSwap);
		}
		else {
			cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
		}

		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFunctionCall);
		break;
	}
	case ExpressionType::kYieldExpression: {
		GenerateExpression(exp->as<YieldExpression>().argument().get());
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kYield);
		break;
	}
	case ExpressionType::kImportExpression: {
		auto& import_exp = exp->as<ImportExpression>();
		auto const_idx = AllocConst(MakeConstValue(import_exp.source().get()));
		cur_func_def_->byte_code().EmitConstLoad(const_idx);
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetModuleAsync);
		break;
	}
	default:
		throw SyntaxError("Unrecognized exp.");
		break;
	}
}

void CodeGener::GeneratorArrayExpression(ArrayExpression* arr_exp) {
	GenerateParamList(arr_exp->elements());

	auto literal_new = AllocConst(Value(ArrayObjectClassDef::LiteralNew));
	cur_func_def_->byte_code().EmitConstLoad(literal_new);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFunctionCall);
}

void CodeGener::GeneratorObjectExpression(ObjectExpression* obj_exp) {
	for (auto& prop : obj_exp->properties()) {
		// 将key和value入栈
		auto key_const_index = AllocConst(Value(String::New(prop.key)));
		cur_func_def_->byte_code().EmitConstLoad(key_const_index);
		GenerateExpression(prop.value.get());
	}
	auto const_idx = AllocConst(Value(obj_exp->properties().size() * 2));
	cur_func_def_->byte_code().EmitConstLoad(const_idx);

	auto literal_new = AllocConst(Value(ObjectClassDef::LiteralNew));
	cur_func_def_->byte_code().EmitConstLoad(literal_new);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFunctionCall);
}

void CodeGener::GenerateFunctionBody(Statement* statement) {
	if (statement->is(StatementType::kBlock)) {
		auto& block = statement->as<BlockStatement>();
		for (size_t i = 0; i < block.statements().size(); i++) {
			auto& stat = block.statements()[i];
			GenerateStatement(stat.get());
			if (i != block.statements().size() - 1) {
				continue;
			}
			if (!stat->is(StatementType::kReturn)) {
				// 补全末尾的return
				cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
				cur_func_def_->byte_code().EmitReturn(cur_func_def_);
			}
		}
	}
	else {
		// 表达式体
		GenerateExpression(statement->as<ExpressionStatement>().expression().get());
		cur_func_def_->byte_code().EmitReturn(cur_func_def_);
	}
	
}

void CodeGener::GenerateFunctionExpression(FunctionExpression* exp) {
	auto const_idx = AllocConst(Value(new FunctionDef(&context_->runtime(), exp->id(), exp->params().size())));
	auto& func_def = GetConstValueByIndex(const_idx).function_def();
	func_def.set_is_normal();
	if (exp->is_generator()) {
		func_def.set_is_generator();
	}
	else if (exp->is_async()) {
		func_def.set_is_async();
	}

	auto load_pc = cur_func_def_->byte_code().Size();
	// 可能需要修复，统一用U32了
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kCLoadD);
	cur_func_def_->byte_code().EmitU32(const_idx);

	if (!exp->id().empty()) {
		// 非匿名函数分配变量来装，这里其实有个没考虑的地方
		// 如果外层还有一层赋值，那么该函数的名字应该只在函数内作用域有效
		auto& var_info = AllocVar(exp->id(), VarFlags::kConst);
		cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);

		if (exp->is_export()) {
			static_cast<ModuleDef*>(cur_func_def_)->AddExportVar(exp->id(), var_info.var_idx);
		}
	}

	// 保存环境，以生成新指令流
	auto savefunc = cur_func_def_;

	// 切换环境
	EntryScope(&func_def, ScopeType::kFunction);
	cur_func_def_ = &func_def;

	// 参数正序分配
	for (size_t i = 0; i < cur_func_def_->par_count(); ++i) {
		AllocVar(exp->params()[i]);
	}

	GenerateFunctionBody(exp->body().get());
	
	bool need_repair = !cur_func_def_->closure_var_defs().empty();

	// 恢复环境
	ExitScope();
	cur_func_def_ = savefunc;

	if (need_repair) {
		cur_func_def_->byte_code().RepairOpcode(load_pc, OpcodeType::kClosure);
	}

}

void CodeGener::GenerateArrowFunctionExpression(ArrowFunctionExpression* exp) {
	auto& arrow_exp = exp->as<ArrowFunctionExpression>();
	auto const_idx = AllocConst(Value(new FunctionDef(&context_->runtime(), "", arrow_exp.params().size())));
	auto& func_def = GetConstValueByIndex(const_idx).function_def();
	func_def.set_is_arrow();
	if (arrow_exp.is_async()) {
		func_def.set_is_async();
	}

	auto load_pc = cur_func_def_->byte_code().Size();
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kCLoadD);
	cur_func_def_->byte_code().EmitU32(const_idx);

	// 保存环境，以生成新指令流
	auto savefunc = cur_func_def_;

	// 切换环境
	EntryScope(&func_def, ScopeType::kArrowFunction);
	cur_func_def_ = &func_def;

	// 参数正序分配
	for (auto& param : arrow_exp.params()) {
		AllocVar(param);
	}

	GenerateFunctionBody(exp->body().get());

	bool need_repair = cur_func_def_->has_this() || !cur_func_def_->closure_var_defs().empty();

	// 恢复环境
	cur_func_def_ = savefunc;
	ExitScope();

	if (need_repair) {
		cur_func_def_->byte_code().RepairOpcode(load_pc, OpcodeType::kClosure);
	}
}

void CodeGener::GenerateLValueStore(Expression* lvalue_exp) {
	if (lvalue_exp->value_category() != ValueCategory::kLValue) {
		throw SyntaxError("The left side of the assignment operator must be an lvalue.");
	}

	// 再处理左值表达式
	switch (lvalue_exp->type()) {
		// 为左值赋值
	case ExpressionType::kIdentifier: {
		const auto* var_info = GetVarByExpression(lvalue_exp);
		assert(var_info);
		if ((var_info->flags & VarFlags::kConst) == VarFlags::kConst) {
			throw SyntaxError("Cannot change const var.");
		}
		cur_func_def_->byte_code().EmitVarStore(var_info->var_idx);
		break;
	}
	case ExpressionType::kMemberExpression: {
		auto& member_exp = lvalue_exp->as<MemberExpression>();

		// 设置对象的属性
		// 如：obj.a.b = 100;
		// 先入栈obj.a这个对象
		GenerateExpression(member_exp.object().get());

		if (member_exp.computed()) {
			GenerateExpression(member_exp.property().get());
			cur_func_def_->byte_code().EmitIndexedStore();
		}
		else {
			auto& prop_exp = member_exp.property()->as<Identifier>();
			auto const_idx = AllocConst(Value(String::New(prop_exp.name())));
			cur_func_def_->byte_code().EmitPropertyStore(const_idx);
		}
		break;
	}
	default:
		throw SyntaxError("Lvalue expression type error.");
	}
}

void CodeGener::GenerateStatement(Statement* stat) {
	auto start_pc = cur_func_def_->byte_code().Size();
	switch (stat->type()) {
	case StatementType::kBlock: {
		GenerateBlock(&stat->as<BlockStatement>());
		break;
	}
	case StatementType::kExpression: {
		GenerateExpressionStatement(&stat->as<ExpressionStatement>());
		break;
	}
	case StatementType::kReturn: {
		GenerateReturnStatement(&stat->as<ReturnStatement>());
		break;
	}
	case StatementType::kVariableDeclaration: {
		GenerateVariableDeclaration(&stat->as<VariableDeclaration>());
		break;
	}
	case StatementType::kIf: {
		GenerateIfStatement(&stat->as<IfStatement>());
		break;
	}
	case StatementType::kWhile: {
		GenerateWhileStatement(&stat->as<WhileStatement>());
		break;
	}
	case StatementType::kFor: {
		GenerateForStatement(&stat->as<ForStatement>());
		break;
	}
	case StatementType::kLabeled: {
		GenerateLabeledStatement(&stat->as<LabeledStatement>());
		break;
	}
	case StatementType::kContinue: {
		GenerateContinueStatement(&stat->as<ContinueStatement>());
		break;
	}
	case StatementType::kBreak: {
		GenerateBreakStatement(&stat->as<BreakStatement>());
		break;
	}
	case StatementType::kTry: {
		GenerateTryStatement(&stat->as<TryStatement>());
		break;
	}
	case StatementType::kThrow: {
		GenerateThrowStatement(&stat->as<ThrowStatement>());
		break;
	}
	case StatementType::kImport: {
		GenerateImportDeclaration(&stat->as<ImportDeclaration>());
		break;
	}
	case StatementType::kExport: {
		GenerateExportDeclaration(&stat->as<ExportDeclaration>());
		break;
	}
	default:
		throw SyntaxError("Unknown statement type");
	}
	auto end_pc = cur_func_def_->byte_code().Size();
	cur_func_def_->debug_table().AddEntry(start_pc, end_pc, stat->start(), stat->end());
}

void CodeGener::GenerateExpressionStatement(ExpressionStatement* stat) {
	auto exp = stat->expression().get();
	// 抛弃纯表达式语句的最终结果
	if (exp) {
		GenerateExpression(exp);
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);
	}
}

void CodeGener::GenerateImportDeclaration(ImportDeclaration* stat) {
	auto const_idx = AllocConst(Value(String::New(stat->source())));
	cur_func_def_->byte_code().EmitConstLoad(const_idx);

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGetModule);

	// 模块对象保存到变量
	auto& var_info = AllocVar(stat->name(), VarFlags::kConst);
	cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);
}

void CodeGener::GenerateExportDeclaration(ExportDeclaration* stat) {
	if (!cur_func_def_->is_module()) {
		throw SyntaxError("Only modules can export.");
	}
	GenerateStatement(stat->declaration().get());
}


void CodeGener::GenerateVariableDeclaration(VariableDeclaration* decl) {
	VarFlags flags = VarFlags::kNone;
	if (decl->kind() == TokenType::kKwConst) {
		flags = VarFlags::kConst;
	}

	auto& var_info = AllocVar(decl->name(), flags);
	GenerateExpression(decl->init().get());
	// 保存到变量中
	cur_func_def_->byte_code().EmitVarStore(var_info.var_idx);
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kPop);

	if (decl->is_export()) {
		static_cast<ModuleDef*>(cur_func_def_)->AddExportVar(decl->name(), var_info.var_idx);
	}
}


void CodeGener::GenerateIfStatement(IfStatement* stat) {
	// 表达式结果压栈
	GenerateExpression(stat->test().get());

	// 条件为false时，跳转到if块之后的地址
	auto if_pc = cur_func_def_->byte_code().Size();
	GenerateIfEq(stat->test().get());

	GenerateBlock(stat->consequent().get());


	if (stat->alternate()) {
		// 跳过当前余下所有else if / else的指令
		auto end_pc = cur_func_def_->byte_code().Size();

		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
		cur_func_def_->byte_code().EmitPcOffset(0);

		// 修复条件为false时，跳转到if块之后的地址
		cur_func_def_->byte_code().RepairPc(if_pc, cur_func_def_->byte_code().Size());

		if (stat->alternate()->is(StatementType::kIf)) {
			GenerateIfStatement(&stat->alternate()->as<IfStatement>());
		}
		else {
			assert(stat->alternate()->is(StatementType::kBlock));
			GenerateBlock(&stat->alternate()->as<BlockStatement>());
		}

		cur_func_def_->byte_code().RepairPc(end_pc, cur_func_def_->byte_code().Size());
	}
	else {
		// 修复条件为false时，跳转到if块之后的地址
		cur_func_def_->byte_code().RepairPc(if_pc, cur_func_def_->byte_code().Size());
	}
}

void CodeGener::GenerateLabeledStatement(LabeledStatement* stat) {
	auto res = label_map_.emplace(stat->label(), LableInfo());
	if (!res.second) {
		throw SyntaxError("Duplicate label.");
	}

	auto save_cur_label_reloop_pc_ = cur_label_reloop_pc_;
	cur_label_reloop_pc_ = kInvalidPc;

	GenerateStatement(stat->body().get());

	RepairEntrys(res.first->second.entrys, cur_func_def_->byte_code().Size(), *cur_label_reloop_pc_);

	label_map_.erase(res.first);

	cur_label_reloop_pc_ = save_cur_label_reloop_pc_;
}


void CodeGener::GenerateForStatement(ForStatement* stat) {
	auto save_cur_loop_repair_entrys = cur_loop_repair_entrys_;

	std::vector<RepairEntry> loop_repair_entrys;
	cur_loop_repair_entrys_ = &loop_repair_entrys;

	EntryScope(nullptr, ScopeType::kFor);

	// init
	GenerateStatement(stat->init().get());

	auto start_pc = cur_func_def_->byte_code().Size();

	// 表达式结果压栈
	if (stat->test()) {
		GenerateExpression(stat->test().get());
	}

	// 等待修复
	loop_repair_entrys.emplace_back(RepairEntry{
		.type = RepairEntry::Type::kBreak,
		.repair_pc = cur_func_def_->byte_code().Size(),
	});
	// 提前写入跳转的指令
	GenerateIfEq(stat->test().get());

	bool need_set_label = cur_label_reloop_pc_ && cur_label_reloop_pc_ == kInvalidPc;
	cur_label_reloop_pc_ = std::nullopt;

	GenerateBlock(stat->body().get(), false);

	// 记录重新循环的pc
	auto reloop_pc = cur_func_def_->byte_code().Size();
	if (need_set_label) {
		cur_label_reloop_pc_ = reloop_pc;
	}

	if (stat->update()) {
		GenerateExpression(stat->update().get());
	}

	ExitScope();

	// 重新回去看是否需要循环
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	cur_func_def_->byte_code().EmitPcOffset(0);
	cur_func_def_->byte_code().RepairPc(cur_func_def_->byte_code().Size() - 3, start_pc);

	RepairEntrys(loop_repair_entrys, cur_func_def_->byte_code().Size(), reloop_pc);

	cur_loop_repair_entrys_ = save_cur_loop_repair_entrys;
}

void CodeGener::GenerateWhileStatement(WhileStatement* stat) {
	auto save_cur_loop_repair_entrys = cur_loop_repair_entrys_;

	std::vector<RepairEntry> loop_repair_entrys;
	cur_loop_repair_entrys_ = &loop_repair_entrys;

	// 记录重新循环的pc
	auto reloop_pc = cur_func_def_->byte_code().Size();
	if (cur_label_reloop_pc_ && cur_label_reloop_pc_ == kInvalidPc) {
		cur_label_reloop_pc_ = reloop_pc;
	}

	// 表达式结果压栈
	GenerateExpression(stat->test().get());

	// 等待修复
	loop_repair_entrys.emplace_back(RepairEntry{
		.type = RepairEntry::Type::kBreak,
		.repair_pc = cur_func_def_->byte_code().Size(),
		});
	// 提前写入跳转的指令
	GenerateIfEq(stat->test().get());

	GenerateBlock(stat->body().get(), true, ScopeType::kWhile);

	// 重新回去看是否需要循环
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	cur_func_def_->byte_code().EmitPcOffset(0);
	cur_func_def_->byte_code().RepairPc(cur_func_def_->byte_code().Size() - 3, reloop_pc);

	RepairEntrys(loop_repair_entrys, cur_func_def_->byte_code().Size(), reloop_pc);

	cur_loop_repair_entrys_ = save_cur_loop_repair_entrys;
}

void CodeGener::GenerateContinueStatement(ContinueStatement* stat) {
	if (cur_loop_repair_entrys_ == nullptr) {
		throw SyntaxError("Cannot use break in acyclic scope");
	}

	if (stat->label()) {
		auto iter = label_map_.find(*stat->label());
		if (iter == label_map_.end()) {
			throw SyntaxError("Label does not exist.");
		}
		iter->second.entrys.emplace_back(RepairEntry{
			.type = RepairEntry::Type::kContinue,
			.repair_pc = cur_func_def_->byte_code().Size(),
		});
	}
	else {
		cur_loop_repair_entrys_->emplace_back(RepairEntry{
			.type = RepairEntry::Type::kContinue,
			.repair_pc = cur_func_def_->byte_code().Size(),
		});
	}

	// 跳到当前循环的末尾pc，等待修复
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction, ScopeType::kArrowFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyGoto);
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	}
	cur_func_def_->byte_code().EmitPcOffset(0);
}

void CodeGener::GenerateBreakStatement(BreakStatement* stat) {
	if (cur_loop_repair_entrys_ == nullptr) {
		throw SyntaxError("Cannot use break in acyclic scope.");
	}

	if (stat->label()) {
		auto iter = label_map_.find(*stat->label());
		if (iter == label_map_.end()) {
			throw SyntaxError("Label does not exist.");
		}
		iter->second.entrys.emplace_back(RepairEntry{
			.type = RepairEntry::Type::kBreak,
			.repair_pc = cur_func_def_->byte_code().Size(),
		});
	}
	else {
		cur_loop_repair_entrys_->emplace_back(RepairEntry{
			.type = RepairEntry::Type::kBreak,
			.repair_pc = cur_func_def_->byte_code().Size(),
		});
	}
	
	// 无法提前得知结束pc，保存待修复pc，等待修复
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction, ScopeType::kArrowFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyGoto);
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	}
	cur_func_def_->byte_code().EmitPcOffset(0);
}


void CodeGener::GenerateReturnStatement(ReturnStatement* stat) {
	if (stat->argument().get()) {
		GenerateExpression(stat->argument().get());
	}
	else {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kUndefined);
	}
	if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kFunction })) {
		cur_func_def_->byte_code().EmitOpcode(OpcodeType::kFinallyReturn);
	}
	else {
		cur_func_def_->byte_code().EmitReturn(cur_func_def_);
	}
}


void CodeGener::GenerateTryStatement(TryStatement* stat) {
	auto has_finally = bool(stat->finalizer());

	auto try_start_pc = cur_func_def_->byte_code().Size();

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kTryBegin);

	GenerateBlock(stat->block().get(), true, has_finally ? ScopeType::kTryFinally : ScopeType::kTry);

	auto try_end_pc = cur_func_def_->byte_code().Size();

	// 这里需要生成跳向finally的指令
	auto repair_end_pc = try_end_pc;
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kGoto);
	cur_func_def_->byte_code().EmitPcOffset(0);

	auto catch_start_pc = kInvalidPc;
	auto catch_end_pc = kInvalidPc;
	auto catch_err_var_idx = kVarInvaildIndex;

	if (stat->handler()) {
		catch_start_pc = cur_func_def_->byte_code().Size();
		EntryScope(nullptr, has_finally ? ScopeType::kCatchFinally : ScopeType::kCatch);

		// 加载error参数到变量
		catch_err_var_idx = AllocVar(stat->handler()->param()->name()).var_idx;

		GenerateBlock(stat->handler()->body().get(), false);

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
	if (stat->finalizer()) {
		finally_start_pc = cur_func_def_->byte_code().Size();
		GenerateBlock(stat->finalizer()->body().get(), true, ScopeType::kFinally);
		finally_end_pc = cur_func_def_->byte_code().Size();
	}

	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kTryEnd);

	if (!stat->handler() && !stat->finalizer()) {
		throw SyntaxError("There cannot be a statement with only try.");
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

void CodeGener::GenerateThrowStatement(ThrowStatement* stat) {
	GenerateExpression(stat->argument().get());
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kThrow);
}


void CodeGener::GenerateBlock(BlockStatement* block, bool entry_scope, ScopeType type) {
	if (entry_scope) {
		EntryScope(nullptr, type);
	}
	for (auto& stat : block->statements()) {
		GenerateStatement(stat.get());
	}
	if (entry_scope) {
		ExitScope();
	}
}


void CodeGener::GenerateIfEq(Expression* exp) {
	cur_func_def_->byte_code().EmitOpcode(OpcodeType::kIfEq);
	cur_func_def_->byte_code().EmitPcOffset(0);
}

void CodeGener::GenerateParamList(const std::vector<std::unique_ptr<Expression>>& par_list) {
	// 参数正序入栈
	for (size_t i = 0; i < par_list.size(); ++i) {
		GenerateExpression(par_list[i].get());
	}

	auto const_idx = AllocConst(Value(par_list.size()));
	cur_func_def_->byte_code().EmitConstLoad(const_idx);
}

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
	return context_->FindConstOrInsertToGlobal(std::move(value));
}

const Value& CodeGener::GetConstValueByIndex(ConstIndex idx) {
	return context_->GetConstValue(idx);
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
			// 在上层函数作用域找到了，构建捕获链
			auto scope_func = scopes_[i].function_def();

			// 途径的每一级作用域，都需要构建
			for (size_t j = i + 1; j < scopes_.size(); ++j) {
				if (scope_func == scopes_[j].function_def()) {
					continue;
				}
				scope_func = scopes_[j].function_def();

				// 为Value(&closure_var)分配变量
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


const VarInfo* CodeGener::GetVarByExpression(Expression* exp) {
	assert(exp->is(ExpressionType::kIdentifier));
	auto& var_exp = exp->as<Identifier>();
	auto var_info = FindVarIndexByName(var_exp.name());
	if (!var_info) {
		return nullptr;
	}
	return var_info;
}

Value CodeGener::MakeConstValue(Expression* exp) {
	switch (exp->type()) {
	case ExpressionType::kUndefined:{
		return Value();
	}
	case ExpressionType::kNull: {
		return Value(nullptr);
	}
	case ExpressionType::kBoolean: {
		return Value(exp->as<BooleanLiteral>().value());
	}
	case ExpressionType::kFloat: {
		return Value(exp->as<FloatLiteral>().value());
	}
	case ExpressionType::kInteger: {
		return Value(exp->as<IntegerLiteral>().value());
	}
	case ExpressionType::kString: {
		return Value(String::New(exp->as<StringLiteral>().value()));
	}
	default:
		throw SyntaxError("Unable to generate expression for value");
	}
}

void CodeGener::RepairEntrys(const std::vector<RepairEntry>& entrys, Pc end_pc, Pc reloop_pc) {
	for (auto& repair_info : entrys) {
		switch (repair_info.type) {
		case RepairEntry::Type::kBreak: {
			cur_func_def_->byte_code().RepairPc(repair_info.repair_pc, end_pc);
			break;
		}
		case RepairEntry::Type::kContinue: {
			assert(reloop_pc != kInvalidPc);
			cur_func_def_->byte_code().RepairPc(repair_info.repair_pc, reloop_pc);
			break;
		}
		default:
			throw SyntaxError("Incorrect type.");
			break;
		}
	}
}

} // namespace compiler
} // namespace mjs
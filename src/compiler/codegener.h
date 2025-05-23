#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <initializer_list>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/function_object.h>

#include "parser.h"
#include "scope.h"
#include "statement.h"
#include "expression.h"

namespace mjs {

class Runtime;

namespace compiler {

class CodeGener : public noncopyable {
public:
	struct RepairEntry {
		enum class Type {
			kBreak,
			kContinue,
		} type;
		Pc repair_pc;
	};

public:
	CodeGener(Context* context, Parser* parser);

	void AddCppFunction(const std::string& func_name, CppFunction func);

	// void AddImportModule();

	Value Generate(std::string&& module_name, std::string_view source);


private:
	void GenerateExpression(Expression* exp);
	void GeneratorArrayExpression(ArrayExpression* arr_exp);
	void GeneratorObjectExpression(ObjectExpression* obj_exp);
	void GenerateFunctionBody(Statement* statement);
	void GenerateFunctionExpression(FunctionExpression* exp);
	void GenerateArrowFunctionExpression(ArrowFunctionExpression* exp);
	void GenerateLValueStore(Expression* lvalue_exp);

	void GenerateStatement(Statement* stat);

	void GenerateExpressionStatement(ExpressionStatement* stat);

	void GenerateImportDeclaration(ImportDeclaration* stat);
	void GenerateExportDeclaration(ExportDeclaration* stat);

	void GenerateVariableDeclaration(VariableDeclaration* stat);

	void GenerateIfStatement(IfStatement* stat);
	void GenerateLabeledStatement(LabeledStatement* stat);

	void GenerateForStatement(ForStatement* stat);
	void GenerateWhileStatement(WhileStatement* stat);
	void GenerateContinueStatement(ContinueStatement* stat);
	void GenerateBreakStatement(BreakStatement* stat);

	void GenerateReturnStatement(ReturnStatement* stat);

	void GenerateTryStatement(TryStatement* stat);
	void GenerateThrowStatement(ThrowStatement* stat);

	void GenerateBlock(BlockStatement* block, bool entry_scope = true, ScopeType type = ScopeType::kNone);
	

	void GenerateIfEq(Expression* exp);
	void GenerateParamList(const std::vector<std::unique_ptr<Expression>>& par_list);


	void EntryScope(FunctionDef* sub_func = nullptr, ScopeType type = ScopeType::kNone);
	void ExitScope();

	ConstIndex AllocConst(Value&& value);
	const Value& GetConstValueByIndex(ConstIndex idx);

	const VarInfo& AllocVar(const std::string& name, VarFlags flags = VarFlags::kNone);
	const VarInfo* FindVarIndexByName(const std::string& name);
	bool IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types);

	const VarInfo* GetVarByExpression(Expression* exp);

	Value MakeConstValue(Expression* exp);

	void RepairEntrys(const std::vector<RepairEntry>& entrys, Pc end_pc, Pc reloop_pc);

private:
	Context* context_;
	Parser* parser_;

	// 模块
	ModuleDef* cur_module_def_ = nullptr;				// 当前生成模块

	// 函数
	FunctionDef* cur_func_def_ = nullptr;				// 当前生成函数

	// 作用域
	std::vector<Scope> scopes_;

	// 循环
	std::vector<RepairEntry>* cur_loop_repair_entrys_ = nullptr;

	struct LableInfo {
		Pc cur_loop_start_pc = kInvalidPc;
		std::vector<RepairEntry> entrys;
	};
	// 每个label保存一个start loop
	// 以及一个cur label
	// for和while开始的时候，判断cur label没有start loop，就会填充
	std::unordered_map<std::string, LableInfo> label_map_;
	std::optional<Pc> cur_label_reloop_pc_;

	// 异常
	bool has_finally_ = false;  // 当前作用域是否关联finally块
};

} // namespace compiler
} // namespace mjs
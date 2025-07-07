/**
 * @file code_generator.cpp
 * @brief 代码生成器实现
 */
#include "code_generator.h"

#include <iostream>
#include <stdexcept>

#include <mjs/error.h>
#include <mjs/context.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>

namespace mjs {
namespace compiler {

CodeGenerator::CodeGenerator(Context* context, Parser* parser)
    : context_(context)
    , parser_(parser) {}

void CodeGenerator::AddCppFunction(const std::string& func_name, CppFunction func) {
    auto& var_info = AllocateVar(func_name, VarFlags::kConst);
    auto const_idx = AllocateConst(Value(func));

    // 生成将函数放到变量表中的代码
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);
    current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

Value CodeGenerator::Generate(std::string&& module_name, std::string_view source) {
    scopes_.clear();

    // 创建模块的函数定义
    current_module_def_ = new ModuleDef(&context_->runtime(), std::move(module_name), source, 0);
    current_func_def_ = current_module_def_;
    current_func_def_->set_is_module();
    AllocateConst(Value(current_func_def_));

    EnterScope();

    // 处理导入声明
    for (auto& decl : parser_->import_declarations()) {
        GenerateStatement(decl.get());
    }

    // 处理其他语句
    for (auto& stat : parser_->statements()) {
        GenerateStatement(stat.get());
    }

    // 生成返回指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    ExitScope();

    // 排序调试表
    current_module_def_->debug_table().Sort();
    return Value(static_cast<ModuleDef*>(current_func_def_));
}

void CodeGenerator::GenerateExpression(Expression* exp) {
    switch (exp->type()) {
    case ExpressionType::kUndefined:
    case ExpressionType::kNull:
    case ExpressionType::kBoolean:
    case ExpressionType::kInteger:
    case ExpressionType::kFloat:
    case ExpressionType::kTemplateElement:
    case ExpressionType::kString: {
        auto const_idx = AllocateConst(MakeConstValue(exp));
        current_func_def_->bytecode_table().EmitConstLoad(const_idx);
        break;
    }
    case ExpressionType::kArrayExpression: {
        GenerateArrayExpression(&exp->as<ArrayExpression>());
        break;
    }
    case ExpressionType::kObjectExpression: {
        GenerateObjectExpression(&exp->as<ObjectExpression>());
        break;
    }
    case ExpressionType::kIdentifier: {
        auto& ident_exp = exp->as<Identifier>();

        //auto class_def = runtime_->class_def_table().find(ident_exp.name());
        //// 先不考虑js里定义的类
        //if (class_def) {
        //	auto const_idx = AllocConst(Value(class_def));
        //	current_func_def_->byte_code().EmitConstLoad(const_idx);
        //}
        //else {

            // 尝试查找到对应的变量索引
            const auto* var_info = GetVarInfoByExpression(exp);
            if (var_info) {
                // 从变量中获取
                current_func_def_->bytecode_table().EmitVarLoad(var_info->var_idx);
            }
            else {
                // 尝试从全局对象获取
                auto const_idx = AllocateConst(Value(String::New(ident_exp.name())));
                current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetGlobal);
                current_func_def_->bytecode_table().EmitI32(const_idx);
            }

        // }
        break;
    }
    case ExpressionType::kThisExpression: {
        current_func_def_->set_has_this(true);
        if (IsInTypeScope({ ScopeType::kFunction }, { ScopeType::kArrowFunction })) {
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetThis);
        }
        else {
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetOuterThis);
        }
        break;
    }
    case ExpressionType::kTemplateLiteral: {
        // 生成字符串拼接
        auto& template_exp = exp->as<TemplateLiteral>();
        if (template_exp.expressions().empty()) {
            auto const_idx = AllocateConst(Value(""));
            current_func_def_->bytecode_table().EmitConstLoad(const_idx);
        }
        size_t i = 0;
        for (auto& exp : template_exp.expressions()) {
            GenerateExpression(exp.get());
            ++i;
            if (i == 1) {
                // 确保有一个字符串
                current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kToString);
                continue;
            }
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kAdd);
        }
        break;
    }
    case ExpressionType::kMemberExpression: {
        auto& mem_exp = exp->as<MemberExpression>();

        // 被访问的表达式，入栈这个表达式
        GenerateExpression(mem_exp.object().get());
        // 判断下是否调用函数，是则dump
        if (mem_exp.is_method_call()) {
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kDump);
        }

        if (mem_exp.computed()) {
            // 用于访问的下标的表达式，入栈这个表达式
            GenerateExpression(mem_exp.property().get());

            // 生成索引访问的指令
            current_func_def_->bytecode_table().EmitIndexedLoad();
        }
        else {
            // 成员访问表达式
            auto& prop_exp = mem_exp.property()->as<Identifier>();

            //if (prop_exp->GetType() != ExpType::kIdentifier) {
            //	throw std::runtime_error("Incorrect right value for attribute access.");
            //}

            // 访问对象成员
            auto const_idx = AllocateConst(Value(String::New(prop_exp.name())));
            current_func_def_->bytecode_table().EmitPropertyLoad(const_idx);
        }
        break;
    }
    case ExpressionType::kFunctionExpression: {
        GenerateFunctionExpression(&exp->as<FunctionExpression>());
        break;
    }
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
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kNeg);
            break;
        case TokenType::kOpPrefixInc:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kInc);
            GenerateLValueStore(unary_exp.argument().get());
            break;
        case TokenType::kOpSuffixInc:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kDump);
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kInc);
            GenerateLValueStore(unary_exp.argument().get());
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
            break;
        default:
            throw std::runtime_error("Unsupported unary operator");
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
        auto& binary_exp = exp->as<BinaryExpression>();

        // 其他二元运算

        // 左右表达式的值入栈
        GenerateExpression(binary_exp.left().get());
        GenerateExpression(binary_exp.right().get());

        // 生成运算指令
        switch (binary_exp.op()) {
        case TokenType::kOpAdd:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kAdd);
            break;
        case TokenType::kOpSub:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kSub);
            break;
        case TokenType::kOpMul:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kMul);
            break;
        case TokenType::kOpDiv:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kDiv);
            break;
        case TokenType::kOpEq:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kEq);
            break;
        case TokenType::kOpNe:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kNe);
            break;
        case TokenType::kOpLt:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kLt);
            break;
        case TokenType::kOpGt:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGt);
            break;
        case TokenType::kOpLe:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kLe);
            break;
        case TokenType::kOpGe:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGe);
            break;
        case TokenType::kSepComma:
            break;
        case TokenType::kOpShiftLeft:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kShl);
            break;
        case TokenType::kOpShiftRight:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kShr);
            break;
        case TokenType::kOpUnsignedShiftRight:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUShr);
            break;
        case TokenType::kOpBitAnd:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kBitAnd);
            break;
        case TokenType::kOpBitOr:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kBitOr);
            break;
        case TokenType::kOpBitXor:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kBitXor);
            break;
        default:
            throw std::runtime_error("Unsupported binary operator");
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
        //		current_func_def_->byte_code().EmitConstLoad(const_idx);
        //	}
        //}
        //else {
        GenerateExpression(new_exp.callee().get());
        //}

        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kNew);
        break;
    }
    case ExpressionType::kCallExpression: {
        auto& call_exp = exp->as<CallExpression>();

        //if (func_call_exp->par_list.size() < const_table_[]->function_def()->par_count) {
        //	throw std::runtime_error("Wrong number of parameters passed during function call");
        //}

        GenerateParamList(call_exp.arguments());
        GenerateExpression(call_exp.callee().get());

        // 将this置于栈顶
        if (call_exp.callee()->is(ExpressionType::kMemberExpression)) {
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kSwap);
        }
        else {
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
        }

        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
        break;
    }
    case ExpressionType::kAwaitExpression:
        GenerateExpression(exp->as<AwaitExpression>().argument().get());
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kAwait);
        break;
    case ExpressionType::kYieldExpression: {
        GenerateExpression(exp->as<YieldExpression>().argument().get());
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kYield);
        break;
    }
    case ExpressionType::kImportExpression: {
        auto& import_exp = exp->as<ImportExpression>();
        GenerateExpression(import_exp.source().get());
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetModuleAsync);
        break;
    }
    default:
        throw std::runtime_error("Unsupported expression type");
    }
}

void CodeGenerator::GenerateArrayExpression(ArrayExpression* array_exp) {
    GenerateParamList(array_exp->elements());

    auto literal_new = AllocateConst(Value(ArrayObjectClassDef::LiteralNew));
    current_func_def_->bytecode_table().EmitConstLoad(literal_new);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
}

void CodeGenerator::GenerateObjectExpression(ObjectExpression* obj_exp) {
    for (auto& prop : obj_exp->properties()) {
        // 将key和value入栈
        auto key_const_index = AllocateConst(Value(String::New(prop.key)));
        current_func_def_->bytecode_table().EmitConstLoad(key_const_index);
        GenerateExpression(prop.value.get());
    }
    auto const_idx = AllocateConst(Value(obj_exp->properties().size() * 2));
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);

    auto literal_new = AllocateConst(Value(ObjectClassDef::LiteralNew));
    current_func_def_->bytecode_table().EmitConstLoad(literal_new);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFunctionCall);
}

void CodeGenerator::GenerateFunctionBody(Statement* statement) {
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
                current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
                current_func_def_->bytecode_table().EmitReturn(current_func_def_);
            }
        }
    }
    else {
        // 表达式体
        GenerateExpression(statement->as<ExpressionStatement>().expression().get());
        current_func_def_->bytecode_table().EmitReturn(current_func_def_);
    }
}


void CodeGenerator::GenerateFunctionExpression(FunctionExpression* exp) {
    // 创建函数定义
    auto func_def = new FunctionDef(current_module_def_, exp->id(), exp->params().size());
    // 将函数定义添加到常量池
    auto const_idx = AllocateConst(Value(func_def));

    // 设置函数属性
    func_def->set_is_normal();
    if (exp->is_generator()) {
        func_def->set_is_generator();
    }
    else if (exp->is_async()) {
        func_def->set_is_async();
    }
    
    auto load_pc = current_func_def_->bytecode_table().Size();
    // 可能需要修复，统一用U32了
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
    current_func_def_->bytecode_table().EmitU32(const_idx);

    if (!exp->id().empty()) {
        // 非匿名函数分配变量来装，这里其实有个没考虑的地方
        // 如果外层还有一层赋值，那么该函数的名字应该只在函数内作用域有效
        auto& var_info = AllocateVar(exp->id(), VarFlags::kConst);
        current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);

        if (exp->is_export()) {
            static_cast<ModuleDef*>(current_func_def_)->export_var_def_table().AddExportVar(exp->id(), var_info.var_idx);
        }
    }

    // 保存环境，以生成新指令流
    auto savefunc = current_func_def_;

    // 切换环境
    EnterScope(func_def, ScopeType::kFunction);
    current_func_def_ = func_def;

    // 参数正序分配
    for (size_t i = 0; i < current_func_def_->par_count(); ++i) {
        AllocateVar(exp->params()[i]);
    }

    GenerateFunctionBody(exp->body().get());

    bool need_repair = !current_func_def_->closure_var_table().closure_var_defs().empty();

    // 恢复环境
    ExitScope();
    current_func_def_->debug_table().Sort();
    current_func_def_ = savefunc;

    if (need_repair) {
        current_func_def_->bytecode_table().RepairOpcode(load_pc, OpcodeType::kClosure);
    }
}

void CodeGenerator::GenerateArrowFunctionExpression(ArrowFunctionExpression* exp) {
    auto func_def = new FunctionDef(current_module_def_, "<anonymous_function>", exp->params().size());
    auto const_idx = AllocateConst(Value(func_def));

    func_def->set_is_arrow();
    if (exp->is_async()) {
        func_def->set_is_async();
    }

    auto load_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
    current_func_def_->bytecode_table().EmitU32(const_idx);

    // 保存环境，以生成新指令流
    auto savefunc = current_func_def_;

    // 切换环境
    EnterScope(func_def, ScopeType::kArrowFunction);
    current_func_def_ = func_def;

    // 参数正序分配
    for (auto& param : exp->params()) {
        AllocateVar(param);
    }

    GenerateFunctionBody(exp->body().get());

    bool need_repair = current_func_def_->has_this() || !current_func_def_->closure_var_table().closure_var_defs().empty();

    // 恢复环境
    current_func_def_->debug_table().Sort();
    current_func_def_ = savefunc;
    ExitScope();

    if (need_repair) {
        current_func_def_->bytecode_table().RepairOpcode(load_pc, OpcodeType::kClosure);
    }
}

void CodeGenerator::GenerateLValueStore(Expression* lvalue_exp) {
    if (lvalue_exp->value_category() != ValueCategory::kLValue) {
        throw std::runtime_error("Expression is not an lvalue");
    }
    
    switch (lvalue_exp->type()) {
    case ExpressionType::kIdentifier: {
        auto& ident_exp = lvalue_exp->as<Identifier>();
        const auto* var_info = FindVarInfoByName(ident_exp.name());
        assert(var_info);
        if ((var_info->flags & VarFlags::kConst) == VarFlags::kConst) {
            throw SyntaxError("Cannot change const var.");
        }
        current_func_def_->bytecode_table().EmitVarStore(var_info->var_idx);
        break;
    }
    case ExpressionType::kMemberExpression: {
        auto& mem_exp = lvalue_exp->as<MemberExpression>();
        
        // 设置对象的属性
        // 如：obj.a.b = 100;
        // 先入栈obj.a这个对象
        GenerateExpression(mem_exp.object().get());
        
        if (mem_exp.computed()) {
            // 计算属性
            GenerateExpression(mem_exp.property().get());
            current_func_def_->bytecode_table().EmitIndexedStore();
        } else {
            // 普通属性
            auto& prop_exp = mem_exp.property()->as<Identifier>();
            auto const_idx = AllocateConst(Value(String::New(prop_exp.name())));
            current_func_def_->bytecode_table().EmitPropertyStore(const_idx);
        }
        break;
    }
    default:
        throw std::runtime_error("Unsupported lvalue expression type");
    }
}

void CodeGenerator::GenerateStatement(Statement* stat) {
    auto start_pc = current_func_def_->bytecode_table().Size();

    switch (stat->type()) {
    case StatementType::kBlock:
        GenerateBlock(&stat->as<BlockStatement>());
        break;
    case StatementType::kExpression:
        GenerateExpressionStatement(&stat->as<ExpressionStatement>());
        break;
    case StatementType::kVariableDeclaration:
        GenerateVariableDeclaration(&stat->as<VariableDeclaration>());
        break;
    case StatementType::kIf:
        GenerateIfStatement(&stat->as<IfStatement>());
        break;
    case StatementType::kLabeled:
        GenerateLabeledStatement(&stat->as<LabeledStatement>());
        break;
    case StatementType::kFor:
        GenerateForStatement(&stat->as<ForStatement>());
        break;
    case StatementType::kWhile:
        GenerateWhileStatement(&stat->as<WhileStatement>());
        break;
    case StatementType::kContinue:
        GenerateContinueStatement(&stat->as<ContinueStatement>());
        break;
    case StatementType::kBreak:
        GenerateBreakStatement(&stat->as<BreakStatement>());
        break;
    case StatementType::kReturn:
        GenerateReturnStatement(&stat->as<ReturnStatement>());
        break;
    case StatementType::kTry:
        GenerateTryStatement(&stat->as<TryStatement>());
        break;
    case StatementType::kThrow:
        GenerateThrowStatement(&stat->as<ThrowStatement>());
        break;
    case StatementType::kImport:
        GenerateImportDeclaration(&stat->as<ImportDeclaration>());
        break;
    case StatementType::kExport:
        GenerateExportDeclaration(&stat->as<ExportDeclaration>());
        break;
    default:
        throw std::runtime_error("Unsupported statement type");
    }

    switch (stat->type()) {
    case StatementType::kBlock:
    case StatementType::kExport:
        break;
    default: {
        auto end_pc = current_func_def_->bytecode_table().Size();
        auto&& [line, column] = current_module_def_->line_table().PosToLineAndColumn(stat->start());
        current_func_def_->debug_table().AddEntry(start_pc, end_pc, stat->start(), stat->end(), line);
    }
    }
}

void CodeGenerator::GenerateExpressionStatement(ExpressionStatement* stat) {
    auto exp = stat->expression().get();
    if (!exp) {
        // 空语句
        return;
    }
    // 抛弃纯表达式语句的最终结果
    GenerateExpression(exp);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

void CodeGenerator::GenerateImportDeclaration(ImportDeclaration* stat) {
    auto source_const_idx = AllocateConst(Value(String::New(stat->source())));

    current_func_def_->bytecode_table().EmitConstLoad(source_const_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetModule);

    auto name_const_idx = AllocateConst(Value(String::New(stat->name())));

    // 模块对象保存到变量
    auto& var_info = AllocateVar(stat->name(), VarFlags::kConst);
    current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

void CodeGenerator::GenerateExportDeclaration(ExportDeclaration* stat) {
    if (!current_func_def_->is_module()) {
        throw SyntaxError("Only modules can export.");
    }

    auto& decl = stat->declaration();
    GenerateStatement(decl.get());
}

void CodeGenerator::GenerateVariableDeclaration(VariableDeclaration* stat) {
    // 确定变量标志
    VarFlags flags = VarFlags::kNone;
    if (stat->kind() == TokenType::kKwConst) {
        flags = VarFlags::kConst;
    }
    
    // 分配变量
    auto& var_info = AllocateVar(stat->name(), flags);
    
    // 如果有初始值，生成初始值代码
    if (stat->init()) {
        GenerateExpression(stat->init().get());
        current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
    }
    
    // 如果是导出变量，添加到模块导出
    if (stat->is_export()) {
        //auto name_const_idx = AllocateConst(Value(String::New(stat->name())));
        //current_func_def_->bytecode_table().EmitVarLoad(var_info.var_idx);
        //// 使用适当的导出操作码
        //current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kExport);
        //current_func_def_->bytecode_table().EmitI32(name_const_idx);

        static_cast<ModuleDef*>(current_func_def_)->export_var_def_table().AddExportVar(stat->name(), var_info.var_idx);
    }
}

void CodeGenerator::GenerateIfStatement(IfStatement* stat) {
    // 表达式结果压栈
    GenerateExpression(stat->test().get());

    // 条件为false时，跳转到if块之后的地址
    auto if_pc = current_func_def_->bytecode_table().Size();
    GenerateIfEq();

    GenerateBlock(stat->consequent().get());


    if (stat->alternate()) {
        // 跳过当前余下所有else if / else的指令
        auto end_pc = current_func_def_->bytecode_table().Size();

        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        current_func_def_->bytecode_table().EmitPcOffset(0);

        // 修复条件为false时，跳转到if块之后的地址
        current_func_def_->bytecode_table().RepairPc(if_pc, current_func_def_->bytecode_table().Size());

        if (stat->alternate()->is(StatementType::kIf)) {
            GenerateIfStatement(&stat->alternate()->as<IfStatement>());
        }
        else {
            assert(stat->alternate()->is(StatementType::kBlock));
            GenerateBlock(&stat->alternate()->as<BlockStatement>());
        }

        current_func_def_->bytecode_table().RepairPc(end_pc, current_func_def_->bytecode_table().Size());
    }
    else {
        // 修复条件为false时，跳转到if块之后的地址
        current_func_def_->bytecode_table().RepairPc(if_pc, current_func_def_->bytecode_table().Size());
    }
}

void CodeGenerator::GenerateLabeledStatement(LabeledStatement* stat) {
    auto res = label_map_.emplace(stat->label(), LabelInfo());
    if (!res.second) {
        throw SyntaxError("Duplicate label.");
    }

    auto save_label_reloop_pc_ = current_label_reloop_pc_;
    current_label_reloop_pc_ = kInvalidPc;

    GenerateStatement(stat->body().get());

    RepairEntries(res.first->second.entries, current_func_def_->bytecode_table().Size(), *current_label_reloop_pc_);

    label_map_.erase(res.first);

    current_label_reloop_pc_ = save_label_reloop_pc_;
}

void CodeGenerator::GenerateForStatement(ForStatement* stat) {
    auto save_loop_repair_entrys = current_loop_repair_entries_;

    std::vector<RepairEntry> loop_repair_entrys;
    current_loop_repair_entries_ = &loop_repair_entrys;

    EnterScope(nullptr, ScopeType::kFor);

    // init
    GenerateStatement(stat->init().get());

    auto start_pc = current_func_def_->bytecode_table().Size();

    // 表达式结果压栈
    if (stat->test()) {
        GenerateExpression(stat->test().get());
    }

    // 等待修复
    loop_repair_entrys.emplace_back(RepairEntry{
        .type = RepairEntry::Type::kBreak,
        .repair_pc = current_func_def_->bytecode_table().Size(),
        });
    // 提前写入跳转的指令
    GenerateIfEq();

    bool need_set_label = current_label_reloop_pc_ && current_label_reloop_pc_ == kInvalidPc;
    current_label_reloop_pc_ = std::nullopt;

    GenerateBlock(stat->body().get(), false);

    // 记录重新循环的pc
    auto reloop_pc = current_func_def_->bytecode_table().Size();
    if (need_set_label) {
        current_label_reloop_pc_ = reloop_pc;
    }

    if (stat->update()) {
        GenerateExpression(stat->update().get());
    }

    ExitScope();

    // 重新回去看是否需要循环
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    current_func_def_->bytecode_table().EmitPcOffset(0);
    current_func_def_->bytecode_table().RepairPc(current_func_def_->bytecode_table().Size() - 3, start_pc);

    RepairEntries(loop_repair_entrys, current_func_def_->bytecode_table().Size(), reloop_pc);

    current_loop_repair_entries_ = save_loop_repair_entrys;
}

void CodeGenerator::GenerateWhileStatement(WhileStatement* stat) {
    auto save_loop_repair_entrys = current_loop_repair_entries_;

    std::vector<RepairEntry> loop_repair_entrys;
    current_loop_repair_entries_ = &loop_repair_entrys;

    // 记录重新循环的pc
    auto reloop_pc = current_func_def_->bytecode_table().Size();
    if (current_label_reloop_pc_ && current_label_reloop_pc_ == kInvalidPc) {
        current_label_reloop_pc_ = reloop_pc;
    }

    // 表达式结果压栈
    GenerateExpression(stat->test().get());

    // 等待修复
    loop_repair_entrys.emplace_back(RepairEntry{
        .type = RepairEntry::Type::kBreak,
        .repair_pc = current_func_def_->bytecode_table().Size(),
        });
    // 提前写入跳转的指令
    GenerateIfEq();

    GenerateBlock(stat->body().get(), true, ScopeType::kWhile);

    // 重新回去看是否需要循环
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    current_func_def_->bytecode_table().EmitPcOffset(0);
    current_func_def_->bytecode_table().RepairPc(current_func_def_->bytecode_table().Size() - 3, reloop_pc);

    RepairEntries(loop_repair_entrys, current_func_def_->bytecode_table().Size(), reloop_pc);

    current_loop_repair_entries_ = save_loop_repair_entrys;
}

void CodeGenerator::GenerateContinueStatement(ContinueStatement* stat) {
    if (current_loop_repair_entries_ == nullptr) {
        throw SyntaxError("Cannot use break in acyclic scope");
    }

    if (stat->label()) {
        auto iter = label_map_.find(*stat->label());
        if (iter == label_map_.end()) {
            throw SyntaxError("Label does not exist.");
        }
        iter->second.entries.emplace_back(RepairEntry{
            .type = RepairEntry::Type::kContinue,
            .repair_pc = current_func_def_->bytecode_table().Size(),
            });
    }
    else {
        current_loop_repair_entries_->emplace_back(RepairEntry{
            .type = RepairEntry::Type::kContinue,
            .repair_pc = current_func_def_->bytecode_table().Size(),
            });
    }

    // 跳到当前循环的末尾pc，等待修复
    if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction, ScopeType::kArrowFunction })) {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFinallyGoto);
    }
    else {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    }
    current_func_def_->bytecode_table().EmitPcOffset(0);
}

void CodeGenerator::GenerateBreakStatement(BreakStatement* stat) {
    if (current_loop_repair_entries_ == nullptr) {
        throw SyntaxError("Cannot use break in acyclic scope.");
    }

    if (stat->label()) {
        auto iter = label_map_.find(*stat->label());
        if (iter == label_map_.end()) {
            throw SyntaxError("Label does not exist.");
        }
        iter->second.entries.emplace_back(RepairEntry{
            .type = RepairEntry::Type::kBreak,
            .repair_pc = current_func_def_->bytecode_table().Size(),
            });
    }
    else {
        current_loop_repair_entries_->emplace_back(RepairEntry{
            .type = RepairEntry::Type::kBreak,
            .repair_pc = current_func_def_->bytecode_table().Size(),
            });
    }

    // 无法提前得知结束pc，保存待修复pc，等待修复
    if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kWhile, ScopeType::kFunction, ScopeType::kArrowFunction })) {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFinallyGoto);
    }
    else {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    }
    current_func_def_->bytecode_table().EmitPcOffset(0);
}

void CodeGenerator::GenerateReturnStatement(ReturnStatement* stat) {
    // 生成返回值
    if (stat->argument()) {
        GenerateExpression(stat->argument().get());
    } else {
        // 无返回值，返回 undefined
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    }
    
    if (IsInTypeScope({ ScopeType::kTryFinally,ScopeType::kCatchFinally, ScopeType::kFinally }, { ScopeType::kFunction })) {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFinallyReturn);
    }
    else {
        current_func_def_->bytecode_table().EmitReturn(current_func_def_);
    }
}

void CodeGenerator::GenerateTryStatement(TryStatement* stat) {
    auto has_finally = bool(stat->finalizer());

    auto try_start_pc = current_func_def_->bytecode_table().Size();

    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kTryBegin);

    GenerateBlock(stat->block().get(), true, has_finally ? ScopeType::kTryFinally : ScopeType::kTry);

    auto try_end_pc = current_func_def_->bytecode_table().Size();

    // 这里需要生成跳向finally的指令
    auto repair_end_pc = try_end_pc;
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    current_func_def_->bytecode_table().EmitPcOffset(0);

    auto catch_start_pc = kInvalidPc;
    auto catch_end_pc = kInvalidPc;
    auto catch_err_var_idx = kVarInvaildIndex;

    if (stat->handler()) {
        catch_start_pc = current_func_def_->bytecode_table().Size();
        EnterScope(nullptr, has_finally ? ScopeType::kCatchFinally : ScopeType::kCatch);

        // 加载error参数到变量
        catch_err_var_idx = AllocateVar(stat->handler()->param()->name()).var_idx;

        GenerateBlock(stat->handler()->body().get(), false);

        ExitScope();
        catch_end_pc = current_func_def_->bytecode_table().Size();
    }
    else {
        catch_end_pc = try_end_pc;
    }

    // 修复pc
    current_func_def_->bytecode_table().RepairPc(repair_end_pc, current_func_def_->bytecode_table().Size());

    // finally是必定会执行的
    auto finally_start_pc = kInvalidPc;
    auto finally_end_pc = kInvalidPc;
    if (stat->finalizer()) {
        finally_start_pc = current_func_def_->bytecode_table().Size();
        GenerateBlock(stat->finalizer()->body().get(), true, ScopeType::kFinally);
        finally_end_pc = current_func_def_->bytecode_table().Size();
    }

    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kTryEnd);

    if (!stat->handler() && !stat->finalizer()) {
        throw SyntaxError("There cannot be a statement with only try.");
    }

    // 添加到异常表
    auto& exception_table = current_func_def_->exception_table();
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

void CodeGenerator::GenerateThrowStatement(ThrowStatement* stat) {
    GenerateExpression(stat->argument().get());
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kThrow);
}

void CodeGenerator::GenerateBlock(BlockStatement* block, bool entry_scope, ScopeType type) {
    if (entry_scope) {
        EnterScope(nullptr, type);
    }

    for (auto& stat : block->statements()) {
        GenerateStatement(stat.get());
    }

    if (entry_scope) {
        ExitScope();
    }
}

void CodeGenerator::GenerateIfEq() {
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
    current_func_def_->bytecode_table().EmitPcOffset(0);
}

void CodeGenerator::GenerateParamList(const std::vector<std::unique_ptr<Expression>>& param_list) {
    // 参数入栈
    for (auto& param : param_list) {
        GenerateExpression(param.get());
    }

    auto const_idx = AllocateConst(Value(param_list.size()));
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);
}

void CodeGenerator::EnterScope(FunctionDefBase* sub_func, ScopeType type) {
    FunctionDefBase* func_def = sub_func ? sub_func : current_func_def_;
    scopes_.emplace_back(func_def, type);
}

void CodeGenerator::ExitScope() {
    scopes_.pop_back();
}

ConstIndex CodeGenerator::AllocateConst(Value&& value) {
    return context_->FindConstOrInsertToGlobal(std::move(value));
}

const Value& CodeGenerator::GetConstValueByIndex(ConstIndex idx) const {
    return context_->GetConstValue(idx);
}

const VarInfo& CodeGenerator::AllocateVar(const std::string& name, VarFlags flags) {
    return scopes_.back().AllocVar(name, flags);
}

const VarInfo* CodeGenerator::FindVarInfoByName(const std::string& name) {
    const VarInfo* find_var_info = nullptr;
    // 就近找变量
    for (ptrdiff_t i = scopes_.size() - 1; i >= 0; --i) {
        // 由于Scope::FindVar不是const方法，我们需要使用非const方式访问
        // 这是一个临时解决方案，最好是修改Scope::FindVar为const方法
        auto var_info = scopes_[i].FindVar(name);
        if (!var_info) {
            // 当前作用域找不到变量，向上层作用域找
            continue;
        }

        auto var_idx = var_info->var_idx;
        if (scopes_[i].function_def() == current_func_def_) {
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
                find_var_info = &scopes_[j].AllocVar(name, var_info->flags);
                scope_func->closure_var_table().AddClosureVar(find_var_info->var_idx, var_idx);
                var_idx = find_var_info->var_idx;
            }
        }
        break;
    }
    return find_var_info;
}

bool CodeGenerator::IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        for (auto type : types) {
            if (it->type() == type) {
                return true;
            }
        }

        for (auto end_type : end_types) {
            if (it->type() == end_type) {
                return false;
            }
        }
    }
    return false;
}

const VarInfo* CodeGenerator::GetVarInfoByExpression(Expression* exp) {
    assert(exp->is(ExpressionType::kIdentifier));
    auto& ident_exp = exp->as<Identifier>();
    return FindVarInfoByName(ident_exp.name());
}

Value CodeGenerator::MakeConstValue(Expression* exp) const {
    switch (exp->type()) {
    case ExpressionType::kUndefined:
        return Value();
    case ExpressionType::kNull:
        return Value(nullptr);
    case ExpressionType::kBoolean:
        return Value(exp->as<BooleanLiteral>().value());
    case ExpressionType::kInteger:
        return Value(exp->as<IntegerLiteral>().value());
    case ExpressionType::kFloat:
        return Value(exp->as<FloatLiteral>().value());
    case ExpressionType::kString:
        return Value(String::New(exp->as<StringLiteral>().value()));
    case ExpressionType::kTemplateElement:
        return Value(String::New(exp->as<TemplateElement>().value()));
    default:
        throw SyntaxError("Unable to generate expression for value");
    }
}

void CodeGenerator::RepairEntries(const std::vector<RepairEntry>& entries, Pc end_pc, Pc reloop_pc) {
    for (auto& repair_info : entries) {
        switch (repair_info.type) {
        case RepairEntry::Type::kBreak: {
            current_func_def_->bytecode_table().RepairPc(repair_info.repair_pc, end_pc);
            break;
        }
        case RepairEntry::Type::kContinue: {
            assert(reloop_pc != kInvalidPc);
            current_func_def_->bytecode_table().RepairPc(repair_info.repair_pc, reloop_pc);
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
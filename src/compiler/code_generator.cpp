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
    , parser_(parser) {
    if (!context) {
        throw std::invalid_argument("Context cannot be null");
    }
    if (!parser) {
        throw std::invalid_argument("Parser cannot be null");
    }
}

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
    if (!exp) {
        throw std::invalid_argument("Expression cannot be null");
    }

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
        case TokenType::kKwAwait:
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kAwait);
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
    case ExpressionType::kBinaryExpression: {
        auto& binary_exp = exp->as<BinaryExpression>();

        // 生成左操作数
        GenerateExpression(binary_exp.left().get());
        // 生成右操作数
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
        //		cur_func_def_->byte_code().EmitConstLoad(const_idx);
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
    if (!array_exp) {
        throw std::invalid_argument("Array expression cannot be null");
    }

    // 创建数组对象
    ClassId array_class_id = ClassId::kArrayObject;
    
    auto const_idx = AllocateConst(Value(static_cast<int32_t>(array_class_id)));
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kNew);
    
    // 添加元素
    for (size_t i = 0; i < array_exp->elements().size(); ++i) {
        // 复制数组对象
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kDump);
        
        // 生成索引
        auto index_const_idx = AllocateConst(Value(static_cast<int64_t>(i)));
        current_func_def_->bytecode_table().EmitConstLoad(index_const_idx);
        
        // 生成值
        GenerateExpression(array_exp->elements()[i].get());
        
        // 存储值
        current_func_def_->bytecode_table().EmitIndexedStore();
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
    }
}

void CodeGenerator::GenerateObjectExpression(ObjectExpression* obj_exp) {
    if (!obj_exp) {
        throw std::invalid_argument("Object expression cannot be null");
    }

    // 创建对象
    ClassId object_class_id = ClassId::kObject;
    
    auto const_idx = AllocateConst(Value(static_cast<int32_t>(object_class_id)));
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kNew);
    
    // 添加属性
    for (const auto& prop : obj_exp->properties()) {
        // 复制对象
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kDump);
        
        // 生成属性名
        auto key_const_idx = AllocateConst(Value(String::New(prop.key)));
        current_func_def_->bytecode_table().EmitConstLoad(key_const_idx);
        
        // 生成属性值
        if (prop.shorthand) {
            // 简写属性，使用属性名作为变量名
            const auto* var_info = FindVarInfoByName(prop.key);
            if (var_info) {
                current_func_def_->bytecode_table().EmitVarLoad(var_info->var_idx);
            }
            else {
                auto name_const_idx = AllocateConst(Value(String::New(prop.key)));
                current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetGlobal);
                current_func_def_->bytecode_table().EmitI32(name_const_idx);
            }
        }
        else if (prop.computed) {
            // 计算属性
            GenerateExpression(prop.value.get());
        }
        else {
            // 普通属性
            GenerateExpression(prop.value.get());
        }
        
        // 存储属性
        current_func_def_->bytecode_table().EmitPropertyStore(key_const_idx);
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
    }
}

void CodeGenerator::EnterScope(FunctionDef* sub_func, ScopeType type) {
    FunctionDef* func_def = sub_func ? sub_func : current_func_def_;
    scopes_.emplace_back(func_def, type);
}

void CodeGenerator::ExitScope() {
    if (scopes_.empty()) {
        throw std::runtime_error("Cannot exit from empty scope stack");
    }
    scopes_.pop_back();
}

ConstIndex CodeGenerator::AllocateConst(Value&& value) {
    return context_->FindConstOrInsertToGlobal(std::move(value));
}

const Value& CodeGenerator::GetConstValueByIndex(ConstIndex idx) const {
    return context_->GetConstValue(idx);
}

const VarInfo& CodeGenerator::AllocateVar(const std::string& name, VarFlags flags) {
    if (scopes_.empty()) {
        throw std::runtime_error("Cannot allocate variable outside of scope");
    }
    return scopes_.back().AllocVar(name, flags);
}

const VarInfo* CodeGenerator::FindVarInfoByName(const std::string& name) const {
    // 从内向外查找变量
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        // 由于Scope::FindVar不是const方法，我们需要使用非const方式访问
        // 这是一个临时解决方案，最好是修改Scope::FindVar为const方法
        auto var_info = it->FindVar(name);
        if (var_info) {
            return var_info;
        }
        
        // 如果是函数作用域，则停止查找
        if (it->type() == ScopeType::kFunction || it->type() == ScopeType::kArrowFunction) {
            break;
        }
    }
    return nullptr;
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

const VarInfo* CodeGenerator::GetVarInfoByExpression(Expression* exp) const {
    if (exp->type() != ExpressionType::kIdentifier) {
        return nullptr;
    }
    
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
        throw std::runtime_error("Cannot make constant value from this expression type");
    }
}

void CodeGenerator::RepairEntries(const std::vector<RepairEntry>& entries, Pc end_pc, Pc reloop_pc) {
    for (auto& entry : entries) {
        switch (entry.type) {
        case RepairEntry::Type::kBreak:
            current_func_def_->bytecode_table().RepairPc(entry.repair_pc, end_pc);
            break;
        case RepairEntry::Type::kContinue:
            current_func_def_->bytecode_table().RepairPc(entry.repair_pc, reloop_pc);
            break;
        }
    }
}

void CodeGenerator::GenerateFunctionExpression(FunctionExpression* exp) {
    if (!exp) {
        throw std::invalid_argument("Function expression cannot be null");
    }

    // 创建函数定义
    auto func_def = new FunctionDef(current_module_def_, exp->id(), exp->start());
    
    // 设置函数属性
    if (exp->is_generator()) {
        func_def->set_is_generator();
    }
    if (exp->is_async()) {
        func_def->set_is_async();
    }
    if (exp->is_module()) {
        func_def->set_is_module();
    }
    
    // 保存当前函数定义
    auto old_func_def = current_func_def_;
    current_func_def_ = func_def;
    
    // 进入函数作用域
    EnterScope(func_def, ScopeType::kFunction);
    
    // 添加参数
    for (const auto& param : exp->params()) {
        AllocateVar(param);
    }
    
    // 生成函数体
    GenerateFunctionBody(exp->body().get());
    
    // 退出函数作用域
    ExitScope();
    
    // 恢复当前函数定义
    current_func_def_ = old_func_def;
    
    // 将函数定义添加到常量池
    auto const_idx = AllocateConst(Value(func_def));
    
    // 生成闭包指令
    current_func_def_->bytecode_table().EmitClosure(const_idx);
}

void CodeGenerator::GenerateArrowFunctionExpression(ArrowFunctionExpression* exp) {
    if (!exp) {
        throw std::invalid_argument("Arrow function expression cannot be null");
    }

    // 创建函数定义
    auto func_def = new FunctionDef(current_module_def_, "", exp->start());
    
    // 设置函数属性
    if (exp->is_async()) {
        func_def->set_is_async();
    }
    func_def->set_is_arrow();
    
    // 保存当前函数定义
    auto old_func_def = current_func_def_;
    current_func_def_ = func_def;
    
    // 进入函数作用域
    EnterScope(func_def, ScopeType::kArrowFunction);
    
    // 添加参数
    for (const auto& param : exp->params()) {
        AllocateVar(param);
    }
    
    // 生成函数体
    if (exp->body()->type() == StatementType::kBlock) {
        GenerateFunctionBody(exp->body().get());
    } else {
        // 箭头函数表达式体
        GenerateExpression(&exp->body()->as<ExpressionStatement>().expression()->as<Expression>());
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kReturn);
    }
    
    // 退出函数作用域
    ExitScope();
    
    // 恢复当前函数定义
    current_func_def_ = old_func_def;
    
    // 将函数定义添加到常量池
    auto const_idx = AllocateConst(Value(func_def));
    
    // 生成闭包指令
    current_func_def_->bytecode_table().EmitClosure(const_idx);
}

void CodeGenerator::GenerateFunctionBody(Statement* statement) {
    if (!statement) {
        throw std::invalid_argument("Function body statement cannot be null");
    }
    
    if (statement->type() == StatementType::kBlock) {
        auto& block = statement->as<BlockStatement>();
        
        // 生成块语句
        for (auto& stat : block.statements()) {
            GenerateStatement(stat.get());
        }
    } else {
        // 单个语句作为函数体
        GenerateStatement(statement);
    }
    
    // 确保函数有返回值
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kReturn);
}

void CodeGenerator::GenerateLValueStore(Expression* lvalue_exp) {
    if (!lvalue_exp) {
        throw std::invalid_argument("LValue expression cannot be null");
    }
    
    if (lvalue_exp->value_category() != ValueCategory::kLValue) {
        throw std::runtime_error("Expression is not an lvalue");
    }
    
    switch (lvalue_exp->type()) {
    case ExpressionType::kIdentifier: {
        auto& ident_exp = lvalue_exp->as<Identifier>();
        const auto* var_info = FindVarInfoByName(ident_exp.name());
        
        if (var_info) {
            // 检查是否为常量
            if ((var_info->flags & VarFlags::kConst) == VarFlags::kConst) {
                throw std::runtime_error("Cannot assign to const variable");
            }
            
            // 存储到变量
            current_func_def_->bytecode_table().EmitVarStore(var_info->var_idx);
        } else {
            // 存储到全局变量
            auto const_idx = AllocateConst(Value(String::New(ident_exp.name())));
            // 使用适当的全局变量设置操作码
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
            current_func_def_->bytecode_table().EmitI32(const_idx);
        }
        break;
    }
    case ExpressionType::kMemberExpression: {
        auto& mem_exp = lvalue_exp->as<MemberExpression>();
        
        // 获取对象
        GenerateExpression(mem_exp.object().get());
        
        if (mem_exp.computed()) {
            // 计算属性
            GenerateExpression(mem_exp.property().get());
            
            // 存储值
            current_func_def_->bytecode_table().EmitIndexedStore();
        } else {
            // 普通属性
            auto& prop_exp = mem_exp.property()->as<Identifier>();
            auto const_idx = AllocateConst(Value(String::New(prop_exp.name())));
            
            // 存储值
            current_func_def_->bytecode_table().EmitPropertyStore(const_idx);
        }
        break;
    }
    default:
        throw std::runtime_error("Unsupported lvalue expression type");
    }
}

void CodeGenerator::GenerateStatement(Statement* stat) {
    if (!stat) {
        throw std::invalid_argument("Statement cannot be null");
    }

    switch (stat->type()) {
    case StatementType::kExpression:
        GenerateExpressionStatement(&stat->as<ExpressionStatement>());
        break;
    case StatementType::kBlock:
        GenerateBlock(&stat->as<BlockStatement>());
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
}

void CodeGenerator::GenerateExpressionStatement(ExpressionStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Expression statement cannot be null");
    }

    // 生成表达式
    GenerateExpression(stat->expression().get());
    
    // 弹出表达式的值
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

void CodeGenerator::GenerateBlock(BlockStatement* block, bool entry_scope, ScopeType type) {
    if (!block) {
        throw std::invalid_argument("Block statement cannot be null");
    }

    // 进入作用域
    if (entry_scope) {
        EnterScope(nullptr, type);
    }
    
    // 生成块中的语句
    for (auto& stat : block->statements()) {
        GenerateStatement(stat.get());
    }
    
    // 退出作用域
    if (entry_scope) {
        ExitScope();
    }
}

void CodeGenerator::GenerateVariableDeclaration(VariableDeclaration* stat) {
    if (!stat) {
        throw std::invalid_argument("Variable declaration cannot be null");
    }

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
    if (!stat) {
        throw std::invalid_argument("If statement cannot be null");
    }

    // 生成条件表达式
    GenerateExpression(stat->test().get());
    
    // 生成条件跳转指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
    auto else_jump_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
    
    // 生成 if 块
    GenerateBlock(stat->consequent().get(), true, ScopeType::kIf);
    
    // 生成跳转指令，跳过 else 块
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    auto end_jump_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
    
    // 修复 else 跳转地址
    auto else_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().RepairPc(else_jump_pc, else_pc);
    
    // 生成 else 块
    if (stat->alternate()) {
        if (stat->alternate()->type() == StatementType::kIf) {
            // else if
            GenerateStatement(stat->alternate().get());
        } else {
            // else
            GenerateBlock(
                stat->alternate()->type() == StatementType::kBlock 
                    ? &stat->alternate()->as<BlockStatement>() 
                    : nullptr,
                true, 
                ScopeType::kElse
            );
        }
    }
    
    // 修复 end 跳转地址
    auto end_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().RepairPc(end_jump_pc, end_pc);
}

void CodeGenerator::GenerateParamList(const std::vector<std::unique_ptr<Expression>>& param_list) {
    // 参数入栈
    for (auto& param : param_list) {
        GenerateExpression(param.get());
    }

    auto const_idx = AllocateConst(Value(param_list.size()));
    current_func_def_->bytecode_table().EmitConstLoad(const_idx);
}

void CodeGenerator::GenerateLabeledStatement(LabeledStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Labeled statement cannot be null");
    }

    // 保存当前标签信息
    auto& label_info = label_map_[stat->label()];
    
    // 生成标签语句
    if (stat->body()->type() == StatementType::kWhile || stat->body()->type() == StatementType::kFor) {
        // 循环语句
        GenerateStatement(stat->body().get());
    } else {
        // 非循环语句
        GenerateStatement(stat->body().get());
        
        // 修复跳转指令
        auto end_pc = current_func_def_->bytecode_table().Size();
        RepairEntries(label_info.entries, end_pc, 0);
        
        // 清空标签信息
        label_info.entries.clear();
    }
}

void CodeGenerator::GenerateForStatement(ForStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("For statement cannot be null");
    }

    // 进入循环作用域
    EnterScope(nullptr, ScopeType::kFor);
    
    // 初始化语句
    if (stat->init()) {
        GenerateStatement(stat->init().get());
    }
    
    // 循环开始位置
    auto loop_start_pc = current_func_def_->bytecode_table().Size();
    
    // 条件表达式
    if (stat->test()) {
        GenerateExpression(stat->test().get());
        
        // 条件跳转，如果条件为假，跳出循环
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
        auto loop_end_jump_pc = current_func_def_->bytecode_table().Size();
        current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
        
        // 循环体
        std::vector<RepairEntry> repair_entrys;
        auto old_repair_entrys = current_loop_repair_entries_;
        current_loop_repair_entries_ = &repair_entrys;
        
        GenerateBlock(stat->body().get(), true, ScopeType::kNone);
        
        // 更新语句位置
        auto update_pc = current_func_def_->bytecode_table().Size();
        
        // 更新语句
        if (stat->update()) {
            GenerateExpression(stat->update().get());
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
        }
        
        // 跳回循环开始
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        current_func_def_->bytecode_table().EmitPcOffset(loop_start_pc - current_func_def_->bytecode_table().Size() - 2);
        
        // 循环结束位置
        auto loop_end_pc = current_func_def_->bytecode_table().Size();
        
        // 修复跳转指令
        current_func_def_->bytecode_table().RepairPc(loop_end_jump_pc, loop_end_pc);
        
        // 修复 break 和 continue 语句
        RepairEntries(repair_entrys, loop_end_pc, update_pc);
        
        // 恢复上一层循环的修复条目
        current_loop_repair_entries_ = old_repair_entrys;
    } else {
        // 无条件循环
        
        // 循环体
        std::vector<RepairEntry> repair_entrys;
        auto old_repair_entrys = current_loop_repair_entries_;
        current_loop_repair_entries_ = &repair_entrys;
        
        GenerateBlock(stat->body().get(), true, ScopeType::kNone);
        
        // 更新语句位置
        auto update_pc = current_func_def_->bytecode_table().Size();
        
        // 更新语句
        if (stat->update()) {
            GenerateExpression(stat->update().get());
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
        }
        
        // 跳回循环开始
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        current_func_def_->bytecode_table().EmitPcOffset(loop_start_pc - current_func_def_->bytecode_table().Size() - 2);
        
        // 循环结束位置（实际上不会执行到这里，除非有 break）
        auto loop_end_pc = current_func_def_->bytecode_table().Size();
        
        // 修复 break 和 continue 语句
        RepairEntries(repair_entrys, loop_end_pc, update_pc);
        
        // 恢复上一层循环的修复条目
        current_loop_repair_entries_ = old_repair_entrys;
    }
    
    // 退出循环作用域
    ExitScope();
}

void CodeGenerator::GenerateWhileStatement(WhileStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("While statement cannot be null");
    }

    // 进入循环作用域
    EnterScope(nullptr, ScopeType::kWhile);
    
    // 循环开始位置
    auto loop_start_pc = current_func_def_->bytecode_table().Size();
    
    // 条件表达式
    GenerateExpression(stat->test().get());
    
    // 条件跳转，如果条件为假，跳出循环
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
    auto loop_end_jump_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
    
    // 循环体
    std::vector<RepairEntry> repair_entrys;
    auto old_repair_entrys = current_loop_repair_entries_;
    current_loop_repair_entries_ = &repair_entrys;
    
    GenerateBlock(stat->body().get(), true, ScopeType::kNone);
    
    // 跳回循环开始
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    current_func_def_->bytecode_table().EmitPcOffset(loop_start_pc - current_func_def_->bytecode_table().Size() - 2);
    
    // 循环结束位置
    auto loop_end_pc = current_func_def_->bytecode_table().Size();
    
    // 修复跳转指令
    current_func_def_->bytecode_table().RepairPc(loop_end_jump_pc, loop_end_pc);
    
    // 修复 break 和 continue 语句
    RepairEntries(repair_entrys, loop_end_pc, loop_start_pc);
    
    // 恢复上一层循环的修复条目
    current_loop_repair_entries_ = old_repair_entrys;
    
    // 退出循环作用域
    ExitScope();
}

void CodeGenerator::GenerateContinueStatement(ContinueStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Continue statement cannot be null");
    }

    if (stat->label()) {
        // 带标签的 continue
        auto it = label_map_.find(*stat->label());
        if (it == label_map_.end()) {
            throw std::runtime_error("Label not found: " + *stat->label());
        }
        
        // 生成跳转指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        auto continue_pc = current_func_def_->bytecode_table().Size();
        current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
        
        // 添加到标签的修复条目
        RepairEntry entry;
        entry.type = RepairEntry::Type::kContinue;
        entry.repair_pc = continue_pc;
        it->second.entries.push_back(entry);
    } else {
        // 普通 continue
        if (!current_loop_repair_entries_) {
            throw std::runtime_error("Continue statement outside of loop");
        }
        
        // 生成跳转指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        auto continue_pc = current_func_def_->bytecode_table().Size();
        current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
        
        // 添加到循环的修复条目
        RepairEntry entry;
        entry.type = RepairEntry::Type::kContinue;
        entry.repair_pc = continue_pc;
        current_loop_repair_entries_->push_back(entry);
    }
}

void CodeGenerator::GenerateBreakStatement(BreakStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Break statement cannot be null");
    }

    if (stat->label()) {
        // 带标签的 break
        auto it = label_map_.find(*stat->label());
        if (it == label_map_.end()) {
            throw std::runtime_error("Label not found: " + *stat->label());
        }
        
        // 生成跳转指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        auto break_pc = current_func_def_->bytecode_table().Size();
        current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
        
        // 添加到标签的修复条目
        RepairEntry entry;
        entry.type = RepairEntry::Type::kBreak;
        entry.repair_pc = break_pc;
        it->second.entries.push_back(entry);
    } else {
        // 普通 break
        if (!current_loop_repair_entries_) {
            throw std::runtime_error("Break statement outside of loop");
        }
        
        // 生成跳转指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
        auto break_pc = current_func_def_->bytecode_table().Size();
        current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
        
        // 添加到循环的修复条目
        RepairEntry entry;
        entry.type = RepairEntry::Type::kBreak;
        entry.repair_pc = break_pc;
        current_loop_repair_entries_->push_back(entry);
    }
}

void CodeGenerator::GenerateReturnStatement(ReturnStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Return statement cannot be null");
    }

    // 生成返回值
    if (stat->argument()) {
        GenerateExpression(stat->argument().get());
    } else {
        // 无返回值，返回 undefined
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    }
    
    // 如果有 finally 块，需要特殊处理
    if (has_finally_) {
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFinallyReturn);
    } else {
        // 生成返回指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kReturn);
    }
}

void CodeGenerator::GenerateTryStatement(TryStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Try statement cannot be null");
    }

    // 生成 try 开始指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kTryBegin);
    auto try_begin_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
    
    // 记录是否有 finally 块
    bool old_has_finally = has_finally_;
    has_finally_ = stat->finalizer() != nullptr;
    
    // 生成 try 块
    GenerateBlock(stat->block().get(), true, ScopeType::kTry);
    
    // 生成 try 结束指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kTryEnd);
    
    // 跳过 catch 和 finally 块
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGoto);
    auto try_end_jump_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().EmitPcOffset(0); // 占位，后面修复
    
    // 修复 try 开始指令
    auto catch_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().RepairPc(try_begin_pc, catch_pc);
    
    // 生成 catch 块
    if (stat->handler()) {
        auto& handler = stat->handler();
        
        // 进入 catch 作用域
        EnterScope(nullptr, has_finally_ ? ScopeType::kCatchFinally : ScopeType::kCatch);
        
        // 分配异常变量
        if (handler->param()) {
            auto& var_info = AllocateVar(handler->param()->name());
            current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);
        } else {
            // 弹出异常值
            current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
        }
        
        // 生成 catch 块
        GenerateBlock(handler->body().get(), false);
        
        // 退出 catch 作用域
        ExitScope();
    }
    
    // 生成 finally 块
    if (stat->finalizer()) {
        auto finally_pc = current_func_def_->bytecode_table().Size();
        
        // 进入 finally 作用域
        EnterScope(nullptr, ScopeType::kFinally);
        
        // 生成 finally 块
        GenerateBlock(stat->finalizer()->body().get(), false);
        
        // 生成 finally 结束指令
        current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kFinallyGoto);
        current_func_def_->bytecode_table().EmitPcOffset(0); // 跳转到下一条指令
        
        // 退出 finally 作用域
        ExitScope();
    }
    
    // 修复 try 结束跳转指令
    auto end_pc = current_func_def_->bytecode_table().Size();
    current_func_def_->bytecode_table().RepairPc(try_end_jump_pc, end_pc);
    
    // 恢复 finally 标志
    has_finally_ = old_has_finally;
}

void CodeGenerator::GenerateThrowStatement(ThrowStatement* stat) {
    if (!stat) {
        throw std::invalid_argument("Throw statement cannot be null");
    }

    // 生成异常值
    GenerateExpression(stat->argument().get());
    
    // 生成抛出指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kThrow);
}

void CodeGenerator::GenerateImportDeclaration(ImportDeclaration* stat) {
    if (!stat) {
        throw std::invalid_argument("Import declaration cannot be null");
    }

    // 生成模块名称常量
    auto source_const_idx = AllocateConst(Value(String::New(stat->source())));
    
    // 生成导入指令
    current_func_def_->bytecode_table().EmitConstLoad(source_const_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kGetModule);
    
    // 生成变量名称常量
    auto name_const_idx = AllocateConst(Value(String::New(stat->name())));
    
    // 分配变量
    auto& var_info = AllocateVar(stat->name(), VarFlags::kConst);
    
    // 存储到变量
    current_func_def_->bytecode_table().EmitVarStore(var_info.var_idx);
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

void CodeGenerator::GenerateExportDeclaration(ExportDeclaration* stat) {
    if (!stat) {
        throw std::invalid_argument("Export declaration cannot be null");
    }

    // 获取导出声明
    auto& decl = stat->declaration();
    
    // 设置导出标志
    if (decl->type() == StatementType::kVariableDeclaration) {
        decl->as<VariableDeclaration>().set_is_export(true);
    } else if (decl->type() == StatementType::kExpression) {
        auto& exp_stat = decl->as<ExpressionStatement>();
        if (exp_stat.expression()->type() == ExpressionType::kFunctionExpression) {
            exp_stat.expression()->as<FunctionExpression>().set_is_export(true);
        }
    }
    
    // 生成声明
    GenerateStatement(decl.get());
}

void CodeGenerator::GenerateIfEq(Expression* exp) {
    if (!exp) {
        throw std::invalid_argument("Expression cannot be null");
    }

    // 生成表达式
    GenerateExpression(exp);
    
    // 生成条件跳转指令
    current_func_def_->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
}

} // namespace compiler
} // namespace mjs 
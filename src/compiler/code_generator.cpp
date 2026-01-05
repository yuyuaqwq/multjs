#include "src/compiler/code_generator.h"

#include <iostream>
#include <stdexcept>

#include <mjs/error.h>
#include <mjs/context.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>

#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/statement_impl/expression_statement.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/member_expression.h"
#include "src/compiler/expression_impl/undefined_literal.h"
#include "src/compiler/expression_impl/null_literal.h"
#include "src/compiler/expression_impl/boolean_literal.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/float_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/template_element.h"

namespace mjs {
namespace compiler {

CodeGenerator::CodeGenerator(Context* context, Parser* parser)
    : context_(context)
    , parser_(parser) {}

void CodeGenerator::AddCppFunction(FunctionDef* function_def, const std::string& func_name, CppFunction func) {
    auto& var_info = scope_manager_.AllocateVar(func_name, VarFlags::kConst);
    auto const_idx = AllocateConst(Value(func));

    // 生成将函数放到变量表中的代码
    function_def->bytecode_table().EmitConstLoad(const_idx);
    function_def->bytecode_table().EmitVarStore(var_info.var_idx);
    function_def->bytecode_table().EmitOpcode(OpcodeType::kPop);
}

Value CodeGenerator::Generate(std::string&& module_name, std::string_view source) {
    scope_manager_.Reset();

    // 创建模块的函数定义
    auto module_def = ModuleDef::New(&context_->runtime(), std::move(module_name), source, 0);
    // current_func_def_ = current_module_def_;
    // current_func_def_->set_is_module();
    module_def->set_is_module();

    AllocateConst(Value(module_def));

    scope_manager_.EnterScope(module_def);

    // 处理导入声明
    for (auto& decl : parser_->import_declarations()) {
        GenerateStatement(module_def, decl.get());
    }

    // 处理其他语句
    for (auto& stat : parser_->statements()) {
        GenerateStatement(module_def, stat.get());
    }

    // 生成返回指令
    module_def->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
    module_def->bytecode_table().EmitOpcode(OpcodeType::kReturn);

    scope_manager_.ExitScope();

    // 排序调试表
    module_def->debug_table().Sort();
    return Value(static_cast<ModuleDef*>(module_def));
}

void CodeGenerator::GenerateExpression(FunctionDefBase* function_def_base, Expression* exp) {
    exp->GenerateCode(this, function_def_base);
}

void CodeGenerator::GenerateStatement(FunctionDefBase* function_def_base, Statement* stat) {
    auto start_pc = function_def_base->bytecode_table().Size();

    if (stat->type() == StatementType::kBlock) {
        scope_manager_.EnterScope(function_def_base, nullptr, ScopeType::kNone);
    }
    stat->GenerateCode(this, function_def_base);
    if (stat->type() == StatementType::kBlock) {
        scope_manager_.ExitScope();
    }

    switch (stat->type()) {
    case StatementType::kBlock:
    case StatementType::kExport:
        break;
    default: {
        auto end_pc = function_def_base->bytecode_table().Size();
        auto&& [line, column] = function_def_base->module_def().line_table().PosToLineAndColumn(stat->start());
        function_def_base->debug_table().AddEntry(start_pc, end_pc, stat->start(), stat->end(), line);
        break;
    }
    }
}

void CodeGenerator::GenerateFunctionBody(FunctionDefBase* function_def_base, Statement* statement) {
    if (statement->is(StatementType::kBlock)) {
        auto& block = statement->as<BlockStatement>();
        for (size_t i = 0; i < block.statements().size(); i++) {
            auto& stat = block.statements()[i];
            GenerateStatement(function_def_base, stat.get());
            if (i != block.statements().size() - 1) {
                continue;
            }
            if (!stat->is(StatementType::kReturn)) {
                // 补全末尾的return
                function_def_base->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
                function_def_base->bytecode_table().EmitReturn(function_def_base);
            }
        }
    }
    else {
        // 表达式体
        GenerateExpression(function_def_base, statement->as<ExpressionStatement>().expression().get());
        function_def_base->bytecode_table().EmitReturn(function_def_base);
    }
}

void CodeGenerator::GenerateLValueStore(FunctionDefBase* function_def_base, Expression* lvalue_exp) {
    if (lvalue_exp->value_category() != ValueCategory::kLValue) {
        throw SyntaxError("Expression is not an lvalue");
    }

    // 使用dynamic_cast来检查表达式类型
    if (auto* ident_exp = dynamic_cast<Identifier*>(lvalue_exp)) {
        const auto* var_info = scope_manager_.FindVarInfoByName(function_def_base, ident_exp->name());
        if (!var_info) {
            throw SyntaxError("Used undefined variables: {}.", ident_exp->name());
        }
        if ((var_info->flags & VarFlags::kConst) == VarFlags::kConst) {
            throw SyntaxError("Cannot change const var.");
        }
        function_def_base->bytecode_table().EmitVarStore(var_info->var_idx);
    }
    else if (auto* mem_exp = dynamic_cast<MemberExpression*>(lvalue_exp)) {
        // 设置对象的属性
        // 如：obj.a.b = 100;
        // 先入栈obj.a这个对象
        GenerateExpression(function_def_base, mem_exp->object().get());

        if (mem_exp->computed()) {
            // 计算属性
            GenerateExpression(function_def_base, mem_exp->property().get());
            function_def_base->bytecode_table().EmitIndexedStore();
        } else {
            // 普通属性
            auto& prop_exp = mem_exp->property()->as<Identifier>();
            auto const_idx = AllocateConst(Value(String::New(prop_exp.name())));
            function_def_base->bytecode_table().EmitPropertyStore(const_idx);
        }
    }
    else {
        throw SyntaxError("Unsupported lvalue expression type");
    }
}

// void CodeGenerator::GenerateBlock(FunctionDefBase* function_def_base, BlockStatement* block, bool entry_scope, ScopeType type) {
//     if (entry_scope) {
//         EnterScope(function_def_base, nullptr, type);
//     }

//     block->GenerateCode(this, function_def_base);

//     if (entry_scope) {
//         ExitScope();
//     }
// }

void CodeGenerator::GenerateIfEq(FunctionDefBase* function_def_base) {
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kIfEq);
    function_def_base->bytecode_table().EmitPcOffset(0);
}

void CodeGenerator::GenerateParamList(FunctionDefBase* function_def_base, const std::vector<std::unique_ptr<Expression>>& param_list) {
    // 参数入栈
    for (auto& param : param_list) {
        GenerateExpression(function_def_base, param.get());
    }

    auto const_idx = AllocateConst(Value(param_list.size()));
    function_def_base->bytecode_table().EmitConstLoad(const_idx);
}


ConstIndex CodeGenerator::AllocateConst(Value&& value) {
    return context_->FindConstOrInsertToGlobal(std::move(value));
}

const Value& CodeGenerator::GetConstValueByIndex(ConstIndex idx) const {
    return context_->GetConstValue(idx);
}

Value CodeGenerator::MakeConstValue(FunctionDefBase* function_def_base, Expression* exp) const {
    if (auto* undefined_exp = dynamic_cast<UndefinedLiteral*>(exp)) {
        return Value();
    } else if (auto* null_exp = dynamic_cast<NullLiteral*>(exp)) {
        return Value(nullptr);
    } else if (auto* bool_exp = dynamic_cast<BooleanLiteral*>(exp)) {
        return Value(bool_exp->value());
    } else if (auto* int_exp = dynamic_cast<IntegerLiteral*>(exp)) {
        return Value(int_exp->value());
    } else if (auto* float_exp = dynamic_cast<FloatLiteral*>(exp)) {
        return Value(float_exp->value());
    } else if (auto* string_exp = dynamic_cast<StringLiteral*>(exp)) {
        return Value(String::New(string_exp->value()));
    } else if (auto* template_element_exp = dynamic_cast<TemplateElement*>(exp)) {
        return Value(String::New(template_element_exp->value()));
    } else {
        throw SyntaxError("Unable to generate expression for value");
    }
}


} // namespace compiler
} // namespace mjs 
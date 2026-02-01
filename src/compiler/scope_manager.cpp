#include "src/compiler/scope_manager.h"

#include <mjs/value/function_def.h>

#include "src/compiler/expression_impl/identifier.h"

namespace mjs {
namespace compiler {

ScopeManager::ScopeManager() = default;

ScopeManager::~ScopeManager() = default;

void ScopeManager::Reset() { 
    scopes_.clear();
}

Scope& ScopeManager::EnterScope(FunctionDefBase* function_def_base, FunctionDefBase* sub_func, ScopeType type) {
    FunctionDefBase* func_def = sub_func ? sub_func : function_def_base;
    scopes_.emplace_back(func_def, type);
    return scopes_.back();
}

void ScopeManager::ExitScope() {
    scopes_.pop_back();
}

const VarInfo& ScopeManager::AllocateVar(const std::string& name, VarFlags flags) {
    return scopes_.back().AllocateVar(name, flags);
}

const VarInfo* ScopeManager::FindVarInfoByName(FunctionDefBase* function_def_base, const std::string& name) {
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
        if (scopes_[i].function_def() == function_def_base) {
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
                find_var_info = &scopes_[j].AllocateVar(name, var_info->flags);
                scope_func->closure_var_table().AddClosureVar(find_var_info->var_idx, var_idx);
                var_idx = find_var_info->var_idx;
            }
        }
        break;
    }
    return find_var_info;
}

bool ScopeManager::IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types) const {
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

const VarInfo* ScopeManager::GetVarInfoByExpression(FunctionDefBase* function_def_base, Expression* exp) {
    auto* ident_exp = dynamic_cast<Identifier*>(exp);
    assert(ident_exp);
    return FindVarInfoByName(function_def_base, ident_exp->name());
}

} // namespace compiler
} // namespace mjs 
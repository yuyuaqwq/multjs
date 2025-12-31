/**
 * @file scope_manager.h
 * @brief 作用域管理器
 */
 
#pragma once

#include <mjs/noncopyable.h>

#include "scope.h"
#include "expression.h"

namespace mjs {
namespace compiler {

/**
 * @class CodeGenerator
 * @brief 代码生成器，将AST转换为字节码
 */
class ScopeManager final : public noncopyable {
public:
    ScopeManager();

    ~ScopeManager();

    void Reset();

    /**
     * @brief 进入作用域
     * @param sub_func 子函数
     * @param type 作用域类型
     */
    Scope& EnterScope(FunctionDefBase* function_def_base, FunctionDefBase* sub_func = nullptr, ScopeType type = ScopeType::kNone);
    
    /**
     * @brief 退出作用域
     */
    void ExitScope();

    /**
     * @brief 分配变量
     * @param name 变量名
     * @param flags 变量标志
     * @return 变量信息
     * @throws std::runtime_error 如果变量名已存在
     */
    const VarInfo& AllocateVar(const std::string& name, VarFlags flags = VarFlags::kNone);
    
    /**
     * @brief 根据名称查找变量索引
     * @param name 变量名
     * @return 变量信息，如果未找到则返回nullptr
     */
    const VarInfo* FindVarInfoByName(FunctionDefBase* function_def_base, const std::string& name);
    
    /**
     * @brief 检查是否在指定类型的作用域中
     * @param types 作用域类型列表
     * @param end_types 结束作用域类型列表
     * @return 是否在指定类型的作用域中
     */
    bool IsInTypeScope(std::initializer_list<ScopeType> types, std::initializer_list<ScopeType> end_types) const;

    /**
     * @brief 根据表达式获取变量信息
     * @param exp 表达式
     * @return 变量信息，如果未找到则返回nullptr
     */
    const VarInfo* GetVarInfoByExpression(FunctionDefBase* function_def_base, Expression* exp);

private:
    std::vector<Scope> scopes_;                     ///< 作用域栈
};

} // namespace compiler
} // namespace mjs 
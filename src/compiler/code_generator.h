/**
 * @file code_generator.h
 * @brief 代码生成器定义
 */
 
#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <initializer_list>
#include <optional>
#include <memory>
#include <stdexcept>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/function_object.h>

#include "./parser.h"
#include "scope.h"
#include "repair_def.h"
#include "jump_manager.h"
#include "statement_impl/block_statement.h"

namespace mjs {

class Runtime;

namespace compiler {

/**
 * @class CodeGenerator
 * @brief 代码生成器，将AST转换为字节码
 */
class CodeGenerator final : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param context 上下文
     * @param parser 解析器
     * @throws std::invalid_argument 如果参数为nullptr
     */
    explicit CodeGenerator(Context* context, Parser* parser);
    
    /**
     * @brief 析构函数
     */
    ~CodeGenerator() = default;
    
    /**
     * @brief 添加C++函数
     * @param func_name 函数名
     * @param func 函数对象
     * @throws std::runtime_error 如果函数名已存在
     */
    void AddCppFunction(FunctionDef* function_def, const std::string& func_name, CppFunction func);

    /**
     * @brief 生成代码
     * @param module_name 模块名
     * @param source 源代码
     * @return 生成的模块值
     */
    Value Generate(std::string&& module_name, std::string_view source);

    /**
     * @brief 获取当前模块定义
     * @return 当前模块定义
     */
    // const ModuleDef* current_module_def() const { return current_module_def_; }

    /**
     * @brief 获取当前模块定义
     * @return 当前模块定义
     */
    // ModuleDef* current_module_def() { return current_module_def_; }

    JumpManager& jump_manager() { return jump_manager_; }
public:
    /**
     * @brief 生成表达式代码
     * @param exp 表达式
     * @throws std::runtime_error 如果表达式类型不支持
     */
    void GenerateExpression(FunctionDefBase* function_def, Expression* exp);

    /**
     * @brief 生成语句代码
     * @param stat 语句
     * @throws std::runtime_error 如果语句类型不支持
     */
    void GenerateStatement(FunctionDefBase* function_def_base, Statement* stat);

    /**
     * @brief 生成函数体代码
     * @param statement 函数体语句
     */
    void GenerateFunctionBody(FunctionDefBase* function_def_base, Statement* statement);

    /**
     * @brief 生成左值存储代码
     * @param lvalue_exp 左值表达式
     * @throws std::runtime_error 如果表达式不是左值
     */
    void GenerateLValueStore(FunctionDefBase* function_def_base, Expression* lvalue_exp);

    /**
     * @brief 生成块语句代码
     * @param block 块语句
     * @param entry_scope 是否进入新作用域
     * @param type 作用域类型
     */
    // void GenerateBlock(FunctionDefBase* function_def_base, BlockStatement* block, bool entry_scope = true, ScopeType type = ScopeType::kNone);
    
    /**
     * @brief 生成条件相等判断代码
     * @param exp 表达式
     */
    void GenerateIfEq(FunctionDefBase* function_def_bas);
    
    /**
     * @brief 生成参数列表代码
     * @param param_list 参数列表
     */
    void GenerateParamList(FunctionDefBase* function_def_base, const std::vector<std::unique_ptr<Expression>>& param_list);

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
     * @brief 分配常量
     * @param value 常量值
     * @return 常量索引
     */
    ConstIndex AllocateConst(Value&& value);
    
    /**
     * @brief 根据索引获取常量值
     * @param idx 常量索引
     * @return 常量值
     * @throws std::out_of_range 如果索引无效
     */
    const Value& GetConstValueByIndex(ConstIndex idx) const;

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

    /**
     * @brief 创建常量值
     * @param exp 表达式
     * @return 常量值
     * @throws SyntaxError 如果表达式类型不支持
     */
    Value MakeConstValue(FunctionDefBase* function_def_base, Expression* exp) const;

private:
    friend class Statement;

    Context* context_;                              ///< 上下文
    Parser* parser_;                                ///< 解析器

    std::vector<Scope> scopes_;                     ///< 作用域栈

    JumpManager jump_manager_;                      ///< 跳转上下文管理器
};

} // namespace compiler
} // namespace mjs 
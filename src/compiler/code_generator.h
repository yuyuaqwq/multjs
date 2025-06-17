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

#include "parser.h"
#include "scope.h"
#include "statement.h"
#include "expression.h"

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
     * @brief 需要修复的跳转指令条目
     */
    struct RepairEntry {
        enum class Type {
            kBreak,    ///< break语句
            kContinue, ///< continue语句
        };
        
        Type type;     ///< 类型
        Pc repair_pc;  ///< 需要修复的PC
    };

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
    void AddCppFunction(const std::string& func_name, CppFunction func);

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
    const ModuleDef* GetCurrentModuleDef() const { return current_module_def_; }

private:
    /**
     * @brief 生成表达式代码
     * @param exp 表达式
     * @throws std::runtime_error 如果表达式类型不支持
     */
    void GenerateExpression(Expression* exp);
    
    /**
     * @brief 生成数组表达式代码
     * @param array_exp 数组表达式
     */
    void GenerateArrayExpression(ArrayExpression* array_exp);
    
    /**
     * @brief 生成对象表达式代码
     * @param object_exp 对象表达式
     */
    void GenerateObjectExpression(ObjectExpression* object_exp);
    
    /**
     * @brief 生成函数体代码
     * @param statement 函数体语句
     */
    void GenerateFunctionBody(Statement* statement);
    
    /**
     * @brief 生成函数表达式代码
     * @param exp 函数表达式
     */
    void GenerateFunctionExpression(FunctionExpression* exp);
    
    /**
     * @brief 生成箭头函数表达式代码
     * @param exp 箭头函数表达式
     */
    void GenerateArrowFunctionExpression(ArrowFunctionExpression* exp);
    
    /**
     * @brief 生成左值存储代码
     * @param lvalue_exp 左值表达式
     * @throws std::runtime_error 如果表达式不是左值
     */
    void GenerateLValueStore(Expression* lvalue_exp);

    /**
     * @brief 生成语句代码
     * @param stat 语句
     * @throws std::runtime_error 如果语句类型不支持
     */
    void GenerateStatement(Statement* stat);

    /**
     * @brief 生成表达式语句代码
     * @param stat 表达式语句
     */
    void GenerateExpressionStatement(ExpressionStatement* stat);

    /**
     * @brief 生成导入声明代码
     * @param stat 导入声明
     */
    void GenerateImportDeclaration(ImportDeclaration* stat);
    
    /**
     * @brief 生成导出声明代码
     * @param stat 导出声明
     */
    void GenerateExportDeclaration(ExportDeclaration* stat);

    /**
     * @brief 生成变量声明代码
     * @param stat 变量声明
     */
    void GenerateVariableDeclaration(VariableDeclaration* stat);

    /**
     * @brief 生成if语句代码
     * @param stat if语句
     */
    void GenerateIfStatement(IfStatement* stat);
    
    /**
     * @brief 生成标签语句代码
     * @param stat 标签语句
     */
    void GenerateLabeledStatement(LabeledStatement* stat);

    /**
     * @brief 生成for语句代码
     * @param stat for语句
     */
    void GenerateForStatement(ForStatement* stat);
    
    /**
     * @brief 生成while语句代码
     * @param stat while语句
     */
    void GenerateWhileStatement(WhileStatement* stat);
    
    /**
     * @brief 生成continue语句代码
     * @param stat continue语句
     */
    void GenerateContinueStatement(ContinueStatement* stat);
    
    /**
     * @brief 生成break语句代码
     * @param stat break语句
     */
    void GenerateBreakStatement(BreakStatement* stat);

    /**
     * @brief 生成return语句代码
     * @param stat return语句
     */
    void GenerateReturnStatement(ReturnStatement* stat);

    /**
     * @brief 生成try语句代码
     * @param stat try语句
     */
    void GenerateTryStatement(TryStatement* stat);
    
    /**
     * @brief 生成throw语句代码
     * @param stat throw语句
     */
    void GenerateThrowStatement(ThrowStatement* stat);

    /**
     * @brief 生成块语句代码
     * @param block 块语句
     * @param entry_scope 是否进入新作用域
     * @param type 作用域类型
     */
    void GenerateBlock(BlockStatement* block, bool entry_scope = true, ScopeType type = ScopeType::kNone);
    
    /**
     * @brief 生成条件相等判断代码
     * @param exp 表达式
     */
    void GenerateIfEq(Expression* exp);
    
    /**
     * @brief 生成参数列表代码
     * @param param_list 参数列表
     */
    void GenerateParamList(const std::vector<std::unique_ptr<Expression>>& param_list);

    /**
     * @brief 进入作用域
     * @param sub_func 子函数
     * @param type 作用域类型
     */
    void EnterScope(FunctionDef* sub_func = nullptr, ScopeType type = ScopeType::kNone);
    
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
    const VarInfo* FindVarInfoByName(const std::string& name) const;
    
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
    const VarInfo* GetVarInfoByExpression(Expression* exp) const;

    /**
     * @brief 创建常量值
     * @param exp 表达式
     * @return 常量值
     * @throws std::runtime_error 如果表达式类型不支持
     */
    Value MakeConstValue(Expression* exp) const;

    /**
     * @brief 修复跳转指令
     * @param entries 需要修复的条目
     * @param end_pc 结束PC
     * @param reloop_pc 重新循环PC
     */
    void RepairEntries(const std::vector<RepairEntry>& entries, Pc end_pc, Pc reloop_pc);

private:
    Context* context_;                              ///< 上下文
    Parser* parser_;                                ///< 解析器

    ModuleDef* current_module_def_ = nullptr;       ///< 当前生成模块
    FunctionDef* current_func_def_ = nullptr;       ///< 当前生成函数

    std::vector<Scope> scopes_;                     ///< 作用域栈

    std::vector<RepairEntry>* current_loop_repair_entries_ = nullptr; ///< 当前循环需要修复的条目

    /**
     * @brief 标签信息
     */
    struct LabelInfo {
        Pc current_loop_start_pc = kInvalidPc;      ///< 当前循环开始PC
        std::vector<RepairEntry> entries;           ///< 需要修复的条目
    };
    
    std::unordered_map<std::string, LabelInfo> label_map_; ///< 标签映射
    std::optional<Pc> current_label_reloop_pc_;     ///< 当前标签重新循环PC

    bool has_finally_ = false;                      ///< 当前作用域是否关联finally块
};

} // namespace compiler
} // namespace mjs 
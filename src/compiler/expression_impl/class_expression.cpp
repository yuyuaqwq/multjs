/**
 * @file class_expression.cpp
 * @brief 类表达式实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include "src/compiler/expression_impl/class_expression.h"

#include <mjs/error.h>
#include <mjs/value.h>
#include <mjs/function_def.h>
#include <mjs/string.h>
#include <mjs/opcode.h>

#include "src/compiler/code_generator.h"
#include "src/compiler/lexer.h"
#include "src/compiler/statement.h"
#include "src/compiler/statement_impl/block_statement.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/function_expression.h"

namespace mjs {
namespace compiler {

void ClassExpression::GenerateCode(CodeGenerator* code_generator, FunctionDefBase* function_def_base) const {
    // Class表达式代码生成
    // class本质上是一个构造函数,其方法定义在prototype上
    auto& scope_manager = code_generator->scope_manager();

    // 1. 查找constructor方法获取参数信息
    std::vector<std::string> constructor_params;
    const FunctionExpression* constructor_func = nullptr;
    const BlockStatement* constructor_body = nullptr;

    // 收集实例字段用于初始化
    std::vector<const ClassElement*> instance_fields;

    for (const auto& element : elements()) {
        if (element.kind() == MethodKind::kConstructor) {
            constructor_func = dynamic_cast<const FunctionExpression*>(element.value().get());
            if (constructor_func) {
                constructor_params = constructor_func->params();
                constructor_body = constructor_func->body().get();
                break;
            }
        } else if (element.kind() == MethodKind::kField) {
            // 收集实例字段
            instance_fields.push_back(&element);
        }
    }

    // 2. 创建构造函数定义
    auto class_name = id().value_or("");
    auto constructor_def = FunctionDef::New(&function_def_base->module_def(), class_name, constructor_params.size());
    constructor_def->set_is_normal();

    // 3. 将构造函数添加到常量池
    auto constructor_const_idx = code_generator->AllocateConst(Value(constructor_def));

    auto load_pc = function_def_base->bytecode_table().Size();
    function_def_base->bytecode_table().EmitOpcode(OpcodeType::kCLoadD);
    function_def_base->bytecode_table().EmitU32(constructor_const_idx);

    // 4. 如果有类名,分配变量存储类引用
    const VarInfo* var_info_ptr = nullptr;
    if (id().has_value()) {
        auto& var_info = scope_manager.AllocateVar(id().value(), VarFlags::kConst);
        var_info_ptr = &var_info;
        function_def_base->bytecode_table().EmitVarStore(var_info.var_idx);
    }

    // 5. 进入构造函数作用域生成代码
    scope_manager.EnterScope(function_def_base, constructor_def, ScopeType::kFunction);

    // 分配构造函数参数
    for (size_t i = 0; i < constructor_def->param_count(); ++i) {
        if (i < constructor_params.size()) {
            scope_manager.AllocateVar(constructor_params[i]);
        } else {
            scope_manager.AllocateVar("");
        }
    }

    // 6. 生成字段初始化代码（在构造函数体之前）
    // 实例字段需要在构造函数开始时初始化
    if (!instance_fields.empty()) {
        // 获取 this
        constructor_def->bytecode_table().EmitOpcode(OpcodeType::kGetThis);

        for (const auto* field : instance_fields) {
            // 生成字段初始值
            field->value()->GenerateCode(code_generator, constructor_def);

            // 生成字段名
            auto field_key_idx = code_generator->AllocateConst(Value(String::New(field->key())));
            constructor_def->bytecode_table().EmitConstLoad(field_key_idx);

            // 重新加载 this
            constructor_def->bytecode_table().EmitOpcode(OpcodeType::kGetThis);

            // 交换栈顶: [this, field_key, value, this] -> [this, field_key, this, value]
            constructor_def->bytecode_table().EmitOpcode(OpcodeType::kSwap);

            // 存储字段: this.field = value
            constructor_def->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
        }
    }

    // 7. 生成构造函数体
    if (constructor_body) {
        code_generator->GenerateFunctionBody(constructor_def, const_cast<BlockStatement*>(constructor_body));
    } else {
        // 没有显式constructor,生成默认构造函数体
        // 如果有继承,需要调用super()
        // 这里暂时生成空函数体
        constructor_def->bytecode_table().EmitOpcode(OpcodeType::kUndefined);
        constructor_def->bytecode_table().EmitReturn(constructor_def);
    }

    bool need_repair = !constructor_def->closure_var_table().closure_var_defs().empty();

    // 8. 退出构造函数作用域
    scope_manager.ExitScope();
    constructor_def->debug_table().Sort();

    // 9. 如果有闭包变量,修复为闭包指令
    if (need_repair) {
        function_def_base->bytecode_table().RepairOpcode(load_pc, OpcodeType::kClosure);
    }

    // 10. 处理类元素(方法、静态字段等)
    for (const auto& element : elements()) {
        if (element.kind() == MethodKind::kConstructor) {
            // 构造函数已经处理
            continue;
        }

        // 处理静态字段
        if (element.kind() == MethodKind::kStaticField) {
            // 静态字段直接设置到构造函数上
            // 重新加载构造函数到栈顶
            if (var_info_ptr) {
                function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
            } else {
                function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
            }

            // 生成字段名
            auto key_const_idx = code_generator->AllocateConst(Value(String::New(element.key())));
            function_def_base->bytecode_table().EmitConstLoad(key_const_idx);

            // 生成字段初始值
            element.value()->GenerateCode(code_generator, function_def_base);

            // 交换栈顶
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
            continue;
        }

        // 跳过实例字段（已经在构造函数中处理）
        if (element.kind() == MethodKind::kField) {
            continue;
        }

        // 重新加载构造函数到栈顶
        if (var_info_ptr) {
            function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
        } else {
            // 匿名类,重新加载常量
            function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
        }

        // 生成属性键
        auto key_const_idx = code_generator->AllocateConst(Value(String::New(element.key())));
        function_def_base->bytecode_table().EmitConstLoad(key_const_idx);

        // 生成属性值(方法)
        element.value()->GenerateCode(code_generator, function_def_base);

        // 交换栈顶: 栈状态变为 [构造函数, 方法, 属性名] -> [构造函数, 属性名, 方法]
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);

        if (element.is_static()) {
            // 静态方法直接设置到构造函数上
            // 栈状态: [构造函数, 属性名, 方法]
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
        } else {
            // 实例方法设置到prototype上
            // 加载prototype对象: 构造函数.prototype
            // 栈状态: [构造函数, 属性名, 方法]
            auto prototype_key_idx = code_generator->AllocateConst(Value(String::New("prototype")));
            function_def_base->bytecode_table().EmitConstLoad(prototype_key_idx);

            // 复制构造函数引用
            if (var_info_ptr) {
                function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
            } else {
                function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
            }

            // 加载prototype属性
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyLoad);

            // 栈状态: [prototype对象, 属性名, 方法]
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
            function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
        }
    }

    // 10. 处理继承关系
    if (has_super_class()) {
        // 设置原型链
        // 需要: Child.__proto__ = Parent
        //      Child.prototype.__proto__ = Parent.prototype

        // 重新加载子类构造函数到栈顶
        if (var_info_ptr) {
            function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
        } else {
            function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
        }

        // 加载父类构造函数
        super_class()->GenerateCode(code_generator, function_def_base);

        // 设置 Child.__proto__ = Parent
        // 栈: [ChildConstructor, ParentConstructor]
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
        auto proto_idx = code_generator->AllocateConst(Value(String::New("__proto__")));
        function_def_base->bytecode_table().EmitConstLoad(proto_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
        // 栈: [ChildConstructor, "__proto__", ParentConstructor]
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);

        // 设置 Child.prototype.__proto__ = Parent.prototype
        // 重新加载子类构造函数
        if (var_info_ptr) {
            function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
        } else {
            function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
        }

        // 加载父类构造函数
        super_class()->GenerateCode(code_generator, function_def_base);

        // 获取 Child.prototype 和 Parent.prototype
        auto prototype_key_idx = code_generator->AllocateConst(Value(String::New("prototype")));
        function_def_base->bytecode_table().EmitConstLoad(prototype_key_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyLoad);
        // 栈: [ChildConstructor, Parent.prototype]

        // 复制 ChildConstructor
        if (var_info_ptr) {
            function_def_base->bytecode_table().EmitVarLoad(var_info_ptr->var_idx);
        } else {
            function_def_base->bytecode_table().EmitConstLoad(constructor_const_idx);
        }

        // 获取 Child.prototype
        function_def_base->bytecode_table().EmitConstLoad(prototype_key_idx);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyLoad);
        // 栈: [ChildConstructor, Parent.prototype, Child.prototype]

        // 设置 Child.prototype.__proto__ = Parent.prototype
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
        auto proto_idx2 = code_generator->AllocateConst(Value(String::New("__proto__")));
        function_def_base->bytecode_table().EmitConstLoad(proto_idx2);
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kSwap);
        // 栈: [Child.prototype, "__proto__", Parent.prototype]
        function_def_base->bytecode_table().EmitOpcode(OpcodeType::kPropertyStore);
    }
}

/**
 * @brief 解析类元素
 *
 * 支持以下几种类元素：
 * - 构造函数: constructor(...) { ... }
 * - 普通方法: methodName(...) { ... }
 * - getter: get propName() { ... }
 * - setter: set propName(value) { ... }
 * - 静态方法: static methodName(...) { ... }
 * - 静态getter/setter: static get propName() { ... }
 * - 类字段: fieldName = initialValue;
 * - 静态字段: static fieldName = initialValue;
 * - 计算属性名: [methodName](...) { ... }
 *
 * @param lexer 词法分析器
 * @param is_static 是否为静态成员
 * @return 解析后的类元素
 */
static ClassElement ParseClassElement(Lexer* lexer, bool is_static) {
    auto start = lexer->GetSourcePosition();

    // 检查是否有async
    bool is_async = false;
    bool has_key = false;  // 标记是否已经解析了key
    std::string key;  // 提前声明key

    if (lexer->PeekToken().is(TokenType::kKwAsync)) {
        // 检查再下一个token是否为方法名或*（PeekTokenN(1)是async，PeekTokenN(2)是method）
        auto next_next_token = lexer->PeekTokenN(2);
        if (next_next_token.is(TokenType::kIdentifier) || next_next_token.is(TokenType::kOpMul) ||
            next_next_token.is(TokenType::kSepLBrack) || next_next_token.is(TokenType::kString) ||
            next_next_token.is(TokenType::kKwGet) || next_next_token.is(TokenType::kKwSet)) {
            lexer->NextToken();  // 消耗async
            is_async = true;
        } else {
            // async 不是修饰符，而是方法名的一部分
            // 此时 next_next_token 应该是 ( 或者其他token，说明 async 是方法名
            if (next_next_token.is(TokenType::kSepLParen)) {
                // async() 方法
                key = "async";
                has_key = true;
                lexer->NextToken();  // 消耗async
            }
        }
    }

    // 检查是否有get/set
    bool is_getter = false;
    bool is_setter = false;

    if (lexer->PeekToken().is(TokenType::kKwGet)) {
        lexer->NextToken();
        is_getter = true;
    } else if (lexer->PeekToken().is(TokenType::kKwSet)) {
        lexer->NextToken();
        is_setter = true;
    }

    // 检查是否有generator (*)
    bool is_generator = false;
    if (lexer->PeekToken().is(TokenType::kOpMul)) {
        lexer->NextToken();
        is_generator = true;
    }

    // 解析属性键
    bool computed = false;
    // std::string key;  // 已经在上面定义

    if (!has_key) {
        if (lexer->PeekToken().is(TokenType::kSepLBrack)) {
            // 计算属性名
            computed = true;
            lexer->NextToken();
            // TODO: 支持任意表达式作为属性名
            if (lexer->PeekToken().is(TokenType::kIdentifier)) {
                key = lexer->MatchToken(TokenType::kIdentifier).value();
            } else if (lexer->PeekToken().is(TokenType::kString)) {
                key = lexer->MatchToken(TokenType::kString).value();
            } else {
                throw SyntaxError("Unsupported computed property name");
            }
            lexer->MatchToken(TokenType::kSepRBrack);
        } else if (lexer->PeekToken().is(TokenType::kIdentifier)) {
            key = lexer->MatchToken(TokenType::kIdentifier).value();
        } else if (lexer->PeekToken().is(TokenType::kString)) {
            key = lexer->MatchToken(TokenType::kString).value();
        } else {
            throw SyntaxError("Expected property name");
        }
    }

    // 检查是否为字段 (有等号)
    if (!is_getter && !is_setter && lexer->PeekToken().is(TokenType::kOpAssign)) {
        // 类字段: fieldName = value;
        lexer->NextToken(); // 消耗 =
        auto value = Expression::ParseExpression(lexer);

        MethodKind kind = is_static ? MethodKind::kStaticField : MethodKind::kField;
        return ClassElement(kind, std::move(key), std::move(value), computed);
    }

    // 检查是否为方法 (有左括号)
    if (!lexer->PeekToken().is(TokenType::kSepLParen)) {
        throw SyntaxError("Expected '(' after method name");
    }

    // 解析方法
    auto method_start = lexer->GetSourcePosition();

    // 解析参数列表
    auto params_res = Expression::TryParseParameters(lexer);
    if (!params_res) {
        throw SyntaxError("Expected parameter list");
    }
    auto params = *params_res;

    // 解析方法体
    auto body = BlockStatement::ParseBlockStatement(lexer);

    auto method_end = lexer->GetRawSourcePosition();

    // 创建方法表达式
    std::string method_id;
    auto method_expr = std::make_unique<FunctionExpression>(
        method_start, method_end,
        std::move(method_id), std::move(params),
        std::move(body),
        is_async, is_generator, false
    );

    // 确定方法类型
    MethodKind kind;
    if (is_static) {
        if (is_getter) {
            kind = MethodKind::kStaticGetter;
        } else if (is_setter) {
            kind = MethodKind::kStaticSetter;
        } else {
            kind = MethodKind::kStatic;
        }
    } else {
        if (key == "constructor") {
            kind = MethodKind::kConstructor;
        } else if (is_getter) {
            kind = MethodKind::kGetter;
        } else if (is_setter) {
            kind = MethodKind::kSetter;
        } else {
            kind = MethodKind::kMethod;
        }
    }

    return ClassElement(kind, std::move(key), std::move(method_expr), computed);
}

/**
 * @brief 解析类体
 *
 * @param lexer 词法分析器
 * @return 类元素列表
 */
static std::vector<ClassElement> ParseClassBody(Lexer* lexer) {
    lexer->MatchToken(TokenType::kSepLCurly);

    std::vector<ClassElement> elements;

    while (!lexer->PeekToken().is(TokenType::kSepRCurly)) {
        // 检查是否有static关键字
        bool is_static = false;
        if (lexer->PeekToken().is(TokenType::kKwStatic)) {
            lexer->NextToken();
            is_static = true;
        }

        // 解析类元素
        auto element = ParseClassElement(lexer, is_static);
        elements.push_back(std::move(element));

        // 可选的分号
        if (lexer->PeekToken().is(TokenType::kSepSemi)) {
            lexer->NextToken();
        }
    }

    lexer->MatchToken(TokenType::kSepRCurly);
    return elements;
}

std::unique_ptr<ClassExpression> ClassExpression::ParseClassExpression(Lexer* lexer, bool force_parse_class_name) {
    auto start = lexer->GetSourcePosition();
    lexer->MatchToken(TokenType::kKwClass);

    // 可选的类名
    std::optional<std::string> id;
    if (force_parse_class_name == true || lexer->PeekToken().is(TokenType::kIdentifier)) {
        id = lexer->MatchToken(TokenType::kIdentifier).value();
    }

    // 可选的继承
    std::unique_ptr<Expression> super_class;
    if (lexer->PeekToken().is(TokenType::kKwExtends)) {
        lexer->NextToken();
        // 继承的父类必须是一个表达式
        // 限制：只能是标识符（暂不支持复杂的表达式）
        if (lexer->PeekToken().is(TokenType::kIdentifier)) {
            auto super_start = lexer->GetSourcePosition();
            auto super_name = lexer->MatchToken(TokenType::kIdentifier).value();
            auto super_end = lexer->GetRawSourcePosition();
            super_class = std::make_unique<Identifier>(super_start, super_end, std::move(super_name));
        } else {
            throw SyntaxError("Super class must be an identifier");
        }
    }

    // 解析类体
    auto elements = ParseClassBody(lexer);

    auto end = lexer->GetRawSourcePosition();
    return std::make_unique<ClassExpression>(
        start, end,
        std::move(id),
        std::move(super_class),
        std::move(elements)
    );
}

} // namespace compiler
} // namespace mjs

/**
 * @file type_inference_engine.cpp
 * @brief TypeInferenceEngine实现
 */

#include "./type_inference_engine.h"
#include "src/compiler/expression.h"
#include "mjs/token.h"
#include "src/compiler/expression_impl/integer_literal.h"
#include "src/compiler/expression_impl/float_literal.h"
#include "src/compiler/expression_impl/string_literal.h"
#include "src/compiler/expression_impl/boolean_literal.h"
#include "src/compiler/expression_impl/identifier.h"
#include "src/compiler/expression_impl/binary_expression.h"
#include "src/compiler/expression_impl/call_expression.h"
#include "src/compiler/expression_impl/member_expression.h"
#include "src/compiler/expression_impl/array_expression.h"
#include "src/compiler/expression_impl/object_expression.h"
#include "src/compiler/expression_impl/function_expression.h"
#include "src/compiler/statement_impl/variable_declaration.h"
#include <stdexcept>
#include <sstream>

namespace mjs {
namespace compiler {
namespace cpp_gen {

TypeInferenceEngine::TypeInferenceEngine() {
    // 创建全局作用域
    EnterScope();
}

std::shared_ptr<CppType> TypeInferenceEngine::InferExpressionType(const Expression* expr) {
    if (!expr) {
        return std::make_shared<CppType>(CppType::Value());
    }

    // 使用RTTI或类型检查来判断表达式类型
    // 这里使用dynamic_cast进行类型检查

    // 尝试各种字面量类型
    if (auto* int_lit = dynamic_cast<const IntegerLiteral*>(expr)) {
        return InferLiteralType(expr);
    }

    if (auto* float_lit = dynamic_cast<const FloatLiteral*>(expr)) {
        return InferLiteralType(expr);
    }

    if (auto* str_lit = dynamic_cast<const StringLiteral*>(expr)) {
        return InferLiteralType(expr);
    }

    if (auto* bool_lit = dynamic_cast<const BooleanLiteral*>(expr)) {
        return InferLiteralType(expr);
    }

    // 标识符
    if (auto* ident = dynamic_cast<const Identifier*>(expr)) {
        return InferIdentifierType(expr);
    }

    // 二元表达式
    if (auto* binary = dynamic_cast<const BinaryExpression*>(expr)) {
        return InferBinaryExpressionType(expr);
    }

    // 函数调用
    if (auto* call = dynamic_cast<const CallExpression*>(expr)) {
        return InferCallExpressionType(expr);
    }

    // 成员访问
    if (auto* member = dynamic_cast<const MemberExpression*>(expr)) {
        return InferMemberExpressionType(expr);
    }

    // 数组表达式
    if (auto* array = dynamic_cast<const ArrayExpression*>(expr)) {
        return InferArrayExpressionType(expr);
    }

    // 对象表达式
    if (auto* object = dynamic_cast<const ObjectExpression*>(expr)) {
        return InferObjectExpressionType(expr);
    }

    // 其他未处理的类型回退到动态类型
    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferLiteralType(const Expression* expr) {
    if (auto* int_lit = dynamic_cast<const IntegerLiteral*>(expr)) {
        (void)int_lit; // unused
        return std::make_shared<CppType>(CppType::Int64());
    }

    if (auto* float_lit = dynamic_cast<const FloatLiteral*>(expr)) {
        (void)float_lit; // unused
        return std::make_shared<CppType>(CppType::Float64());
    }

    if (auto* str_lit = dynamic_cast<const StringLiteral*>(expr)) {
        (void)str_lit; // unused
        return std::make_shared<CppType>(CppType::String());
    }

    if (auto* bool_lit = dynamic_cast<const BooleanLiteral*>(expr)) {
        (void)bool_lit; // unused
        return std::make_shared<CppType>(CppType::Boolean());
    }

    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferIdentifierType(const Expression* expr) {
    auto* ident = dynamic_cast<const Identifier*>(expr);
    if (!ident) {
        return std::make_shared<CppType>(CppType::Value());
    }

    const std::string& name = ident->name();

    // 从作用域栈中查找变量类型
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto var_it = it->find(name);
        if (var_it != it->end()) {
            return var_it->second;
        }
    }

    // 未找到变量声明，回退到动态类型
    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferBinaryExpressionType(const Expression* expr) {
    auto* binary = dynamic_cast<const BinaryExpression*>(expr);
    if (!binary) {
        return std::make_shared<CppType>(CppType::Value());
    }

    auto left_type = InferExpressionType(binary->left().get());
    auto right_type = InferExpressionType(binary->right().get());

    TokenType op = binary->op();

    // 比较运算符总是返回bool
    switch (op) {
        case TokenType::kOpLt:
        case TokenType::kOpGt:
        case TokenType::kOpLe:
        case TokenType::kOpGe:
        case TokenType::kOpEq:
        case TokenType::kOpNe:
        case TokenType::kOpStrictEq:
        case TokenType::kOpStrictNe:
            return std::make_shared<CppType>(CppType::Boolean());

        default:
            break;
    }

    // 逻辑运算符返回bool
    switch (op) {
        case TokenType::kOpAnd:
        case TokenType::kOpOr:
            return std::make_shared<CppType>(CppType::Boolean());

        default:
            break;
    }

    // 算术运算符：合并操作数类型
    switch (op) {
        case TokenType::kOpAdd:
        case TokenType::kOpSub:
        case TokenType::kOpMul:
        case TokenType::kOpDiv:
        case TokenType::kOpMod:
        {
            auto merged = left_type->Merge(*right_type);
            return std::make_shared<CppType>(merged);
        }

        default:
            break;
    }

    // 其他运算符回退到动态类型
    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferCallExpressionType(const Expression* expr) {
    // 函数调用的类型推断需要查找函数签名
    // 这里简化处理：回退到动态类型
    (void)expr; // unused
    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferMemberExpressionType(const Expression* expr) {
    auto* member = dynamic_cast<const MemberExpression*>(expr);
    if (!member) {
        return std::make_shared<CppType>(CppType::Value());
    }

    // 推断对象的类型
    auto object_type = InferExpressionType(member->object().get());

    // 如果对象是结构体类型，尝试获取属性类型
    if (object_type->IsObject()) {
        // 如果是计算属性（object[property]），无法静态推断
        if (member->computed()) {
            return std::make_shared<CppType>(CppType::Value());
        }

        // 获取属性名
        if (auto* prop_ident = dynamic_cast<const Identifier*>(member->property().get())) {
            const std::string& prop_name = prop_ident->name();

            // 在结构体属性中查找
            const auto& properties = object_type->GetObjectProperties();
            for (const auto& prop : properties) {
                if (prop.name == prop_name) {
                    return prop.type;
                }
            }
        }
    }

    // 回退到动态类型
    return std::make_shared<CppType>(CppType::Value());
}

std::shared_ptr<CppType> TypeInferenceEngine::InferArrayExpressionType(const Expression* expr) {
    auto* array = dynamic_cast<const ArrayExpression*>(expr);
    if (!array) {
        return std::make_shared<CppType>(CppType::Value());
    }

    // 获取数组元素
    const auto& elements = array->elements();

    if (elements.empty()) {
        // 空数组，默认为动态类型的数组
        return std::make_shared<CppType>(
            CppType::Array(std::make_shared<CppType>(CppType::Value()))
        );
    }

    // 推断元素类型
    std::shared_ptr<CppType> element_type = nullptr;
    for (const auto& elem : elements) {
        auto elem_type = InferExpressionType(elem.get());
        if (!element_type) {
            element_type = elem_type;
        } else {
            // 合并元素类型
            auto merged = element_type->Merge(*elem_type);
            element_type = std::make_shared<CppType>(merged);
        }
    }

    return std::make_shared<CppType>(CppType::Array(element_type));
}

std::shared_ptr<CppType> TypeInferenceEngine::InferObjectExpressionType(const Expression* expr) {
    auto* object = dynamic_cast<const ObjectExpression*>(expr);
    if (!object) {
        return std::make_shared<CppType>(CppType::Value());
    }

    const auto& properties = object->properties();

    if (properties.empty()) {
        // 空对象，回退到动态类型
        return std::make_shared<CppType>(CppType::Value());
    }

    // 推断每个属性的类型
    std::vector<ObjectPropertyType> property_types;
    for (const auto& prop : properties) {
        auto prop_type = InferExpressionType(prop.value.get());
        property_types.push_back({prop.key, prop_type});
    }

    // 检查是否已经存在具有相同属性的对象类型
    for (const auto& existing_type : object_types_) {
        if (!existing_type->IsObject()) {
            continue;
        }

        const auto& existing_props = existing_type->GetObjectProperties();

        // 检查属性数量是否相同
        if (existing_props.size() != property_types.size()) {
            continue;
        }

        // 检查每个属性是否匹配
        bool match = true;
        for (size_t i = 0; i < property_types.size(); ++i) {
            if (existing_props[i].name != property_types[i].name) {
                match = false;
                break;
            }
            if (!existing_props[i].type->Equals(*property_types[i].type)) {
                match = false;
                break;
            }
        }

        if (match) {
            // 找到匹配的类型，直接返回
            return existing_type;
        }
    }

    // 没有找到匹配的类型，创建新的对象类型
    // 生成唯一的结构体名称
    std::ostringstream struct_name;
    struct_name << "Struct_" << struct_counter_++;
    std::string name = struct_name.str();

    // 创建对象类型
    auto object_type = std::make_shared<CppType>(
        CppType::Object(name, property_types)
    );

    // 将对象类型添加到列表中，用于后续生成结构体定义
    object_types_.push_back(object_type);

    return object_type;
}

void TypeInferenceEngine::InferStatementType(const Statement* stmt) {
    if (!stmt) {
        return;
    }

    // 处理变量声明语句
    if (auto* var_decl = dynamic_cast<const VariableDeclaration*>(stmt)) {
        if (var_decl->init()) {
            // 推断初始化表达式的类型
            auto type = InferExpressionType(var_decl->init().get());
            // 将变量类型存储到当前作用域
            SetVariableType(var_decl->name(), type);
        } else {
            // 没有初始化表达式，使用动态类型
            SetVariableType(var_decl->name(), std::make_shared<CppType>(CppType::Value()));
        }
    }

    // TODO: 处理其他语句类型
    // - if语句：需要进入各个分支进行推断
    // - while/for循环：需要推断循环体
    // - 函数声明：需要推断函数签名
}

FunctionSignature TypeInferenceEngine::InferFunctionSignature(const FunctionExpression* func) {
    FunctionSignature sig;

    if (!func) {
        return sig;
    }

    // 进入函数作用域
    EnterScope();

    // 推断参数类型：默认为动态类型
    // 如果有函数体的使用信息，可以推断更精确的类型
    const auto& params = func->params();
    for (const auto& param : params) {
        // 默认参数类型为动态类型
        auto param_type = std::make_shared<CppType>(CppType::Value());
        sig.param_types.push_back(param_type);
        // 将参数添加到函数作用域
        SetVariableType(param, param_type);
    }

    // 推断返回值类型
    // 遍历函数体，查找所有return语句，合并它们的类型
    auto return_type = std::make_shared<CppType>(CppType::Void()); // 默认void

    if (func->body()) {
        // 遍历函数体查找return语句
        // 这里简化处理：如果有return语句，使用其参数的类型
        // 更精确的实现需要递归分析AST

        // 暂时使用动态类型作为返回值
        return_type = std::make_shared<CppType>(CppType::Value());
    }

    sig.return_type = return_type;

    // 离开函数作用域
    ExitScope();

    // 缓存函数签名
    const std::string& func_id = func->id();
    if (!func_id.empty()) {
        function_signatures_[func_id] = sig;
    }

    return sig;
}

std::shared_ptr<CppType> TypeInferenceEngine::GetVariableType(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto var_it = it->find(name);
        if (var_it != it->end()) {
            return var_it->second;
        }
    }
    return nullptr;
}

void TypeInferenceEngine::SetVariableType(const std::string& name, std::shared_ptr<CppType> type) {
    if (scopes_.empty()) {
        return;
    }

    scopes_.back()[name] = type;
}

void TypeInferenceEngine::EnterScope() {
    scopes_.push_back({});
}

void TypeInferenceEngine::ExitScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool TypeInferenceEngine::SolveConstraints() {
    // TODO: 实现类型约束求解
    // 处理类型变量的统一和约束满足
    return true;
}

void TypeInferenceEngine::Clear() {
    scopes_.clear();
    function_signatures_.clear();
    object_types_.clear();
    struct_counter_ = 0;
    EnterScope(); // 重新创建全局作用域
}

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs

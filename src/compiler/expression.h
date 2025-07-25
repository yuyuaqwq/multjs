#pragma once

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/source.h>
#include "token.h"

namespace mjs {
namespace compiler {

enum class ValueCategory {
    kLValue,
    kRValue,
};

enum class ExpressionType {
    // 基础字面量
    kUndefined,
    kNull,
    kBoolean,
    kInteger,
    kFloat,
    kString,
    kThisExpression,

    // 数据结构
    kArrayExpression,
    kObjectExpression,

    // 函数/类相关
    kFunctionExpression,
    kArrowFunctionExpression,
    kClassExpression,

    // 模板字符串
    kTemplateLiteral,
    kTemplateElement,
    kTaggedTemplateExpression,

    // 成员访问与调用
    kMemberExpression,
    kCallExpression,
    kNewExpression,

    // 运算表达式
    kUnaryExpression,
    kBinaryExpression,
    kAssignmentExpression,
    kConditionalExpression,

    // 控制流相关
    kYieldExpression,
    kAwaitExpression,

    // 模块相关
    kImportExpression,

    // 标识符（放在最后，因为它是引用其他表达式的基础）
    kIdentifier,
};

class Expression : public noncopyable {
public:
    Expression(SourcePos start, SourcePos end)
        : start_(start), end_(end) {}
    virtual ~Expression() = default;

    virtual ExpressionType type() const noexcept = 0;

    bool is(ExpressionType type) const {
        return type == this->type();
    }

    template<typename T>
    T& as() {
        return *static_cast<T*>(this);
    }

    template<typename T>
    const T& as() const {
        return *static_cast<const T*>(this);
    }

    ValueCategory value_category() const { return value_category_; }
    void set_value_category(ValueCategory category) {
        value_category_ = category;
    }
    SourcePos start() const { return start_; }
    SourcePos end() const { return end_; }
    
    /**
     * @brief 克隆表达式
     * @return 克隆的表达式指针
     */
    virtual Expression* clone() const = 0;

private:
    ValueCategory value_category_ = ValueCategory::kRValue;
    SourcePos start_;
    SourcePos end_;
};


class UndefinedLiteral : public Expression {
public:
    UndefinedLiteral(SourcePos start, SourcePos end)
        : Expression(start, end) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kUndefined;
    }
    
    Expression* clone() const override;
};

class NullLiteral : public Expression {
public:
    NullLiteral(SourcePos start, SourcePos end)
        : Expression(start, end) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kNull;
    }
    
    Expression* clone() const override;
};

class BooleanLiteral : public Expression {
public:
    BooleanLiteral(SourcePos start, SourcePos end, bool value)
        : Expression(start, end), value_(value) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kBoolean;
    }

    bool value() const { return value_; }
    
    Expression* clone() const override;

private:
    bool value_;
};

class IntegerLiteral : public Expression {
public:
    IntegerLiteral(SourcePos start, SourcePos end, int64_t value)
        : Expression(start, end), value_(value) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kInteger;
    }

    int64_t value() const { return value_; }
    
    Expression* clone() const override;

private:
    int64_t value_;
};

class FloatLiteral : public Expression {
public:
    FloatLiteral(SourcePos start, SourcePos end, double value)
        : Expression(start, end), value_(value) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kFloat;
    }

    double value() const { return value_; }
    
    Expression* clone() const override;

private:
    double value_;
};

class StringLiteral : public Expression {
public:
    StringLiteral(SourcePos start, SourcePos end, std::string&& value)
        : Expression(start, end), value_(std::move(value)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kString;
    }
    
    const std::string& value() const { return value_; }
    
    Expression* clone() const override;

private:
    std::string value_;
};

class ThisExpression : public Expression {
public:
    ThisExpression(SourcePos start, SourcePos end)
        : Expression(start, end) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kThisExpression;
    }
    
    Expression* clone() const override;
};


class TemplateElement : public Expression {
public:
    TemplateElement(SourcePos start, SourcePos end, std::string&& value)
        : Expression(start, end), value_(std::move(value)) {
    }

    ExpressionType type() const noexcept override {
        return ExpressionType::kTemplateElement;
    }
    
    const std::string& value() const { return value_; }
    
    Expression* clone() const override;

private:
    std::string value_;
};

class TemplateLiteral : public Expression {
public:
    TemplateLiteral(SourcePos start, SourcePos end
        // , std::vector<std::unique_ptr<TemplateElement>>&& quasis
        , std::vector<std::unique_ptr<Expression>>&& expressions)
        : Expression(start, end)
        // , quasis_(std::move(quasis))
        , expressions_(std::move(expressions)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kTemplateLiteral;
    }

    // const std::vector<std::unique_ptr<TemplateElement>>& quasis() const { return quasis_; }
    const std::vector<std::unique_ptr<Expression>>& expressions() const { return expressions_; }
    
    Expression* clone() const override;

private:
    // std::vector<std::unique_ptr<TemplateElement>> quasis_;
    std::vector<std::unique_ptr<Expression>> expressions_;
};



class ArrayExpression : public Expression {
public:
    ArrayExpression(SourcePos start, SourcePos end,
                std::vector<std::unique_ptr<Expression>>&& elements)
        : Expression(start, end), elements_(std::move(elements)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kArrayExpression;
    }

    const std::vector<std::unique_ptr<Expression>>& elements() const { return elements_; }
    
    Expression* clone() const override;

private:
    std::vector<std::unique_ptr<Expression>> elements_;
};

class ObjectExpression : public Expression {
public:
    struct Property {
        std::string key;
        std::unique_ptr<Expression> value;
        bool shorthand;
        bool computed;
    };

    ObjectExpression(SourcePos start, SourcePos end, std::vector<Property>&& properties)
        : Expression(start, end), properties_(std::move(properties)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kObjectExpression;
    }

    const std::vector<Property>& properties() const { return properties_; }
    
    Expression* clone() const override;

private:
    std::vector<Property> properties_;
};


class BlockStatement;
class FunctionExpression : public Expression {
public:
    FunctionExpression(SourcePos start, SourcePos end,
                std::string id, std::vector<std::string>&& params,
                std::unique_ptr<BlockStatement> body, 
                bool is_generator, bool is_async, bool is_module)
        : Expression(start, end), id_(std::move(id)),
        params_(std::move(params)), body_(std::move(body)),
        is_export_(0), is_generator_(is_generator),
        is_async_(is_async), is_module_(is_module) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kFunctionExpression;
    }

    const std::string& id() const { return id_; }
    const std::vector<std::string>& params() const { return params_; }
    const std::unique_ptr<BlockStatement>& body() const { return body_; }
    bool is_generator() const { return is_generator_; }
    bool is_async() const { return is_async_; }
    bool is_module() const { return is_module_; }
    bool is_export() const { return is_export_; }
    void set_is_export(bool is_export) { is_export_ = is_export; }
    
    Expression* clone() const override;

private:
    std::string id_;
    std::vector<std::string> params_;
    std::unique_ptr<BlockStatement> body_;

    struct {
        uint32_t is_export_ : 1;
        uint32_t is_generator_ : 1;
        uint32_t is_async_ : 1;
        uint32_t is_module_ : 1;
    };

};

class Statement;
class ArrowFunctionExpression : public Expression {
public:
    ArrowFunctionExpression(SourcePos start, SourcePos end,
                std::vector<std::string>&& params,
                std::unique_ptr<Statement> body,
                bool is_async)
        : Expression(start, end), params_(std::move(params)),
          body_(std::move(body)), is_async_(is_async) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kArrowFunctionExpression;
    }

    const std::vector<std::string>& params() const { return params_; }
    const std::unique_ptr<Statement>& body() const { return body_; }
    bool is_async() const { return is_async_; }
    
    Expression* clone() const override;

private:
    std::vector<std::string> params_;
    std::unique_ptr<Statement> body_;
    bool is_async_;
};


class MemberExpression : public Expression {
public:
    MemberExpression(SourcePos start, SourcePos end,
                std::unique_ptr<Expression> object,
                std::unique_ptr<Expression> property,
                bool is_method_call, bool computed, bool optional)
        : Expression(start, end), object_(std::move(object)),
        property_(std::move(property)), computed_(computed), 
        is_method_call_(is_method_call), optional_(optional) {
        //if (!computed) {
            set_value_category(ValueCategory::kLValue);
        //}
    }

    ExpressionType type() const noexcept override {
        return ExpressionType::kMemberExpression;
    }

    const std::unique_ptr<Expression>& object() const { return object_; }
    const std::unique_ptr<Expression>& property() const { return property_; }
    bool is_method_call() const { return is_method_call_; }
    bool computed() const { return computed_; }
    bool optional() const { return optional_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> object_;
    std::unique_ptr<Expression> property_;
    bool computed_;
    bool is_method_call_;
    bool optional_;
};

class CallExpression : public Expression {
public:
    CallExpression(SourcePos start, SourcePos end, 
                std::unique_ptr<Expression> callee,
                std::vector<std::unique_ptr<Expression>>&& arguments)
        : Expression(start, end), callee_(std::move(callee)),
        arguments_(std::move(arguments)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kCallExpression;
    }

    const std::unique_ptr<Expression>& callee() const { return callee_; }
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> callee_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};

class NewExpression : public Expression {
public:
    NewExpression(SourcePos start, SourcePos end, 
                std::unique_ptr<Expression> callee,
                std::vector<std::unique_ptr<Expression>>&& arguments)
        : Expression(start, end), callee_(std::move(callee)), 
          arguments_(std::move(arguments)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kNewExpression;
    }

    const std::unique_ptr<Expression>& callee() const { return callee_; }
    const std::vector<std::unique_ptr<Expression>>& arguments() const { return arguments_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> callee_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};



class UnaryExpression : public Expression {
public:
    /**
     * @brief 构造一元表达式
     * 
     * @param start 表达式起始位置
     * @param end 表达式结束位置
     * @param op 运算符类型
     * @param argument 操作数表达式
     * @param is_prefix 是否为前缀运算符（true为前缀，false为后缀）
     */
    UnaryExpression(SourcePos start, SourcePos end,
                TokenType op, std::unique_ptr<Expression> argument,
                bool is_prefix = true)
        : Expression(start, end), operator_(op), argument_(std::move(argument)),
          is_prefix_(is_prefix) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kUnaryExpression;
    }

    /**
     * @brief 获取运算符类型
     * @return 运算符类型
     */
    TokenType op() const { return operator_; }
    
    /**
     * @brief 获取操作数表达式
     * @return 操作数表达式的常量引用
     */
    const std::unique_ptr<Expression>& argument() const { return argument_; }
    
    /**
     * @brief 判断是否为前缀运算符
     * @return 如果是前缀运算符则返回true，否则返回false
     */
    bool is_prefix() const { return is_prefix_; }
    
    Expression* clone() const override;

private:
    TokenType operator_;                   ///< 运算符类型
    std::unique_ptr<Expression> argument_; ///< 操作数表达式
    bool is_prefix_;                       ///< 是否为前缀运算符
};

class BinaryExpression : public Expression {
public:
    BinaryExpression(SourcePos start, SourcePos end,
                TokenType op, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
        : Expression(start, end), operator_(op), left_(std::move(left)), right_(std::move(right)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kBinaryExpression;
    }

    TokenType op() const { return operator_; }
    const std::unique_ptr<Expression>& left() const { return left_; }
    const std::unique_ptr<Expression>& right() const { return right_; }
    
    Expression* clone() const override;

private:
    TokenType operator_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

class AssignmentExpression : public Expression {
public:
    AssignmentExpression(SourcePos start, SourcePos end,
        TokenType op, std::unique_ptr<Expression> left,
        std::unique_ptr<Expression> right)
        : Expression(start, end), operator_(op), left_(std::move(left)), right_(std::move(right)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kAssignmentExpression;
    }

    TokenType op() const { return operator_; }
    const std::unique_ptr<Expression>& left() const { return left_; }
    const std::unique_ptr<Expression>& right() const { return right_; }
    
    Expression* clone() const override;

private:
    TokenType operator_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

class ConditionalExpression : public Expression {
public:
    ConditionalExpression(SourcePos start, SourcePos end,
                std::unique_ptr<Expression> test,
                std::unique_ptr<Expression> consequent,
                std::unique_ptr<Expression> alternate)
        : Expression(start, end), test_(std::move(test)),
        consequent_(std::move(consequent)), alternate_(std::move(alternate)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kConditionalExpression;
    }

    const std::unique_ptr<Expression>& test() const { return test_; }
    const std::unique_ptr<Expression>& consequent() const { return consequent_; }
    const std::unique_ptr<Expression>& alternate() const { return alternate_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> test_;
    std::unique_ptr<Expression> consequent_;
    std::unique_ptr<Expression> alternate_;
};


class YieldExpression : public Expression {
public:
    YieldExpression(SourcePos start, SourcePos end, 
                std::unique_ptr<Expression> argument)
        : Expression(start, end), argument_(std::move(argument)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kYieldExpression;
    }

    const std::unique_ptr<Expression>& argument() const { return argument_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> argument_;
};

class AwaitExpression : public Expression {
public:
    AwaitExpression(SourcePos start, SourcePos end,
        std::unique_ptr<Expression> argument)
        : Expression(start, end), argument_(std::move(argument)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kAwaitExpression;
    }

    const std::unique_ptr<Expression>& argument() const { return argument_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> argument_;
};


class ImportExpression : public Expression {
public:
    ImportExpression(SourcePos start, SourcePos end, std::unique_ptr<Expression> source)
        : Expression(start, end), source_(std::move(source)) {}

    ExpressionType type() const noexcept override {
        return ExpressionType::kImportExpression;
    }

    const std::unique_ptr<Expression>& source() const { return source_; }
    
    Expression* clone() const override;

private:
    std::unique_ptr<Expression> source_;
};


class Identifier : public Expression {
public:
    Identifier(SourcePos start, SourcePos end, std::string&& name)
        : Expression(start, end), name_(std::move(name)) {
        set_value_category(ValueCategory::kLValue);
    }

    ExpressionType type() const noexcept override {
        return ExpressionType::kIdentifier;
    }

    const std::string& name() const { return name_; }
    
    Expression* clone() const override;

private:
    std::string name_;
};


} // namespace compiler
} // namespace mjs
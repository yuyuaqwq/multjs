#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/function_def.h>
#include <mjs/class_def.h>

#include "token.h"
#include "expression.h"

namespace mjs {
namespace compiler {

enum class StatementType {
    // 模块相关
    kImport,
    kExport,
    
    // 声明语句
    kVariableDeclaration,
    
    // 控制流
    kIf,
    kLabeled,
    
    // 循环及控制
    kFor,
    kWhile,
    kContinue,
    kBreak,
    
    // 函数控制
    kReturn,
    
    // 异常处理
    kTry,
    kCatch,
    kFinally,
    kThrow,
    
    // 基本语句
    kExpression,
    kBlock,

    // 类型
    kTypeAnnotation,
    kPredefinedType,
    kNamedType,
    kLiteralType,
    kUnionType,
    kFunctionType,
};

class Statement : public noncopyable {
public:
    Statement(SourcePos start, SourcePos end)
        : start_(start), end_(end) {}

    virtual ~Statement() = default;

    virtual StatementType type() const noexcept = 0;

    bool is(StatementType type) const {
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

    SourcePos start() const { return start_; }
    SourcePos end() const { return end_; }
    
    /**
     * @brief 克隆语句
     * @return 克隆的语句指针
     */
    virtual Statement* clone() const = 0;

private:
    SourcePos start_;
    SourcePos end_;
};


class BlockStatement : public Statement {
public:
    BlockStatement(SourcePos start, SourcePos end, std::vector<std::unique_ptr<Statement>>&& statements)
        : Statement(start, end), statements_(std::move(statements)) {}

    StatementType type() const noexcept override { return StatementType::kBlock; }

    const std::vector<std::unique_ptr<Statement>>& statements() const { return statements_; }

    Statement* clone() const override {
        std::vector<std::unique_ptr<Statement>> cloned_statements;
        for (const auto& stmt : statements_) {
            cloned_statements.push_back(std::unique_ptr<Statement>(stmt->clone()));
        }
        return new BlockStatement(start(), end(), std::move(cloned_statements));
    }

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};


class ImportDeclaration : public Statement {
public:
    ImportDeclaration(SourcePos start, SourcePos end,
        std::string source,
        std::string name)
        : Statement(start, end),
        source_(std::move(source)),
        name_(std::move(name)) {}

    StatementType type() const noexcept override { return StatementType::kImport; }

    const std::string& source() const { return source_; }
    const std::string& name() const { return name_; }
    
    Statement* clone() const override {
        return new ImportDeclaration(start(), end(), source_, name_);
    }

private:
    std::string source_;
    std::string name_;
};

class ExportDeclaration : public Statement {
public:
    explicit ExportDeclaration(SourcePos start, SourcePos end,
        std::unique_ptr<Statement> declaration)
        : Statement(start, end), declaration_(std::move(declaration)) {}

    StatementType type() const noexcept override { return StatementType::kExport; }

    const std::unique_ptr<Statement>& declaration() const { return declaration_; }
    
    Statement* clone() const override {
        return new ExportDeclaration(start(), end(), 
            std::unique_ptr<Statement>(declaration_->clone()));
    }

private:
    std::unique_ptr<Statement> declaration_;
};


class VariableDeclaration : public Statement {
public:
    VariableDeclaration(SourcePos start, SourcePos end,
        std::string name,
        std::unique_ptr<Expression> init,
        TokenType kind)
        : Statement(start, end),
        name_(std::move(name)),
        init_(std::move(init)),
        kind_(kind) {
        is_export_ = false;
    }

    StatementType type() const noexcept override { return StatementType::kVariableDeclaration; }

    const std::string& name() const { return name_; }
    const std::unique_ptr<Expression>& init() const { return init_; }
    TokenType kind() const { return kind_; }
    bool is_export() const { return is_export_; }
    void set_is_export(bool is_export) { is_export_ = is_export; }
    
    Statement* clone() const override {
        auto result = new VariableDeclaration(start(), end(), name_, 
            init_ ? std::unique_ptr<Expression>(static_cast<Expression*>(init_->clone())) : nullptr, 
            kind_);
        result->set_is_export(is_export_);
        return result;
    }

private:
    std::string name_;
    std::unique_ptr<Expression> init_;
    TokenType kind_;

    struct {
        uint32_t is_export_ : 1;
    };
};


class IfStatement : public Statement {
public:
    IfStatement(SourcePos start, SourcePos end,
        std::unique_ptr<Expression> test,
        std::unique_ptr<BlockStatement> consequent,
        std::unique_ptr<Statement> alternate)
        : Statement(start, end),
        test_(std::move(test)),
        consequent_(std::move(consequent)),
        alternate_(std::move(alternate)) {}

    StatementType type() const noexcept override { return StatementType::kIf; }

    const std::unique_ptr<Expression>& test() const { return test_; }
    const std::unique_ptr<BlockStatement>& consequent() const { return consequent_; }
    const std::unique_ptr<Statement>& alternate() const { return alternate_; }
    
    Statement* clone() const override {
        return new IfStatement(start(), end(), 
            std::unique_ptr<Expression>(static_cast<Expression*>(test_->clone())),
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(consequent_->clone())),
            alternate_ ? std::unique_ptr<Statement>(alternate_->clone()) : nullptr);
    }

private:
    std::unique_ptr<Expression> test_;
    std::unique_ptr<BlockStatement> consequent_;
    std::unique_ptr<Statement> alternate_;
};

class LabeledStatement : public Statement {
public:
    LabeledStatement(SourcePos start, SourcePos end,
        std::string label,
        std::unique_ptr<Statement> body)
        : Statement(start, end),
        label_(std::move(label)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kLabeled; }

    const std::string& label() const { return label_; }
    const std::unique_ptr<Statement>& body() const { return body_; }
    
    Statement* clone() const override {
        return new LabeledStatement(start(), end(), label_, 
            std::unique_ptr<Statement>(body_->clone()));
    }

private:
    std::string label_;
    std::unique_ptr<Statement> body_;
};


class ForStatement : public Statement {
public:
    ForStatement(SourcePos start, SourcePos end,
        std::unique_ptr<Statement> init,
        std::unique_ptr<Expression> test,
        std::unique_ptr<Expression> update,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        init_(std::move(init)),
        test_(std::move(test)),
        update_(std::move(update)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kFor; }

    const std::unique_ptr<Statement>& init() const { return init_; }
    const std::unique_ptr<Expression>& test() const { return test_; }
    const std::unique_ptr<Expression>& update() const { return update_; }
    const std::unique_ptr<BlockStatement>& body() const { return body_; }
    
    Statement* clone() const override {
        return new ForStatement(start(), end(),
            init_ ? std::unique_ptr<Statement>(init_->clone()) : nullptr,
            test_ ? std::unique_ptr<Expression>(static_cast<Expression*>(test_->clone())) : nullptr,
            update_ ? std::unique_ptr<Expression>(static_cast<Expression*>(update_->clone())) : nullptr,
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(body_->clone())));
    }

private:
    std::unique_ptr<Statement> init_;
    std::unique_ptr<Expression> test_;
    std::unique_ptr<Expression> update_;
    std::unique_ptr<BlockStatement> body_;
};

class WhileStatement : public Statement {
public:
    WhileStatement(SourcePos start, SourcePos end,
        std::unique_ptr<Expression> test,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        test_(std::move(test)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kWhile; }

    const std::unique_ptr<Expression>& test() const { return test_; }
    const std::unique_ptr<BlockStatement>& body() const { return body_; }
    
    Statement* clone() const override {
        return new WhileStatement(start(), end(),
            std::unique_ptr<Expression>(static_cast<Expression*>(test_->clone())),
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(body_->clone())));
    }

private:
    std::unique_ptr<Expression> test_;
    std::unique_ptr<BlockStatement> body_;
};

class ContinueStatement : public Statement {
public:
    explicit ContinueStatement(SourcePos start, SourcePos end,
        std::optional<std::string> label)
        : Statement(start, end), label_(std::move(label)) {}

    StatementType type() const noexcept override { return StatementType::kContinue; }

    const std::optional<std::string>& label() const { return label_; }
    
    Statement* clone() const override {
        return new ContinueStatement(start(), end(), label_);
    }

private:
    std::optional<std::string> label_;
};

class BreakStatement : public Statement {
public:
    explicit BreakStatement(SourcePos start, SourcePos end,
        std::optional<std::string> label)
        : Statement(start, end), label_(std::move(label)) {}

    StatementType type() const noexcept override { return StatementType::kBreak; }

    const std::optional<std::string>& label() const { return label_; }
    
    Statement* clone() const override {
        return new BreakStatement(start(), end(), label_);
    }

private:
    std::optional<std::string> label_;
};


class ReturnStatement : public Statement {
public:
    explicit ReturnStatement(SourcePos start, SourcePos end,
        std::unique_ptr<Expression> argument)
        : Statement(start, end), argument_(std::move(argument)) {}

    StatementType type() const noexcept override { return StatementType::kReturn; }

    const std::unique_ptr<Expression>& argument() const { return argument_; }
    
    Statement* clone() const override {
        return new ReturnStatement(start(), end(),
            argument_ ? std::unique_ptr<Expression>(static_cast<Expression*>(argument_->clone())) : nullptr);
    }

private:
    std::unique_ptr<Expression> argument_;
};

class CatchClause : public Statement {
public:
    CatchClause(SourcePos start, SourcePos end,
        std::unique_ptr<Identifier> param,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end),
        param_(std::move(param)),
        body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kCatch; }

    const std::unique_ptr<Identifier>& param() const { return param_; }
    const std::unique_ptr<BlockStatement>& body() const { return body_; }
    
    Statement* clone() const override {
        return new CatchClause(start(), end(),
            param_ ? std::unique_ptr<Identifier>(static_cast<Identifier*>(param_->clone())) : nullptr,
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(body_->clone())));
    }

private:
    std::unique_ptr<Identifier> param_;
    std::unique_ptr<BlockStatement> body_;
};

class FinallyClause : public Statement {
public:
    explicit FinallyClause(SourcePos start, SourcePos end,
        std::unique_ptr<BlockStatement> body)
        : Statement(start, end), body_(std::move(body)) {}

    StatementType type() const noexcept override { return StatementType::kFinally; }

    const std::unique_ptr<BlockStatement>& body() const { return body_; }
    
    Statement* clone() const override {
        return new FinallyClause(start(), end(),
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(body_->clone())));
    }

private:
    std::unique_ptr<BlockStatement> body_;
};

class TryStatement : public Statement {
public:
    TryStatement(SourcePos start, SourcePos end,
        std::unique_ptr<BlockStatement> block,
        std::unique_ptr<CatchClause> handler,
        std::unique_ptr<FinallyClause> finalizer)
        : Statement(start, end),
        block_(std::move(block)),
        handler_(std::move(handler)),
        finalizer_(std::move(finalizer)) {}

    StatementType type() const noexcept override { return StatementType::kTry; }

    const std::unique_ptr<BlockStatement>& block() const { return block_; }
    const std::unique_ptr<CatchClause>& handler() const { return handler_; }
    const std::unique_ptr<FinallyClause>& finalizer() const { return finalizer_; }
    
    Statement* clone() const override {
        return new TryStatement(start(), end(),
            std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(block_->clone())),
            handler_ ? std::unique_ptr<CatchClause>(static_cast<CatchClause*>(handler_->clone())) : nullptr,
            finalizer_ ? std::unique_ptr<FinallyClause>(static_cast<FinallyClause*>(finalizer_->clone())) : nullptr);
    }

private:
    std::unique_ptr<BlockStatement> block_;
    std::unique_ptr<CatchClause> handler_;
    std::unique_ptr<FinallyClause> finalizer_;
};

class ThrowStatement : public Statement {
public:
    explicit ThrowStatement(SourcePos start, SourcePos end,
                          std::unique_ptr<Expression> argument)
        : Statement(start, end), argument_(std::move(argument)) {}

    StatementType type() const noexcept override { return StatementType::kThrow; }

    const std::unique_ptr<Expression>& argument() const { return argument_; }
    
    Statement* clone() const override {
        return new ThrowStatement(start(), end(),
            std::unique_ptr<Expression>(static_cast<Expression*>(argument_->clone())));
    }

private:
    std::unique_ptr<Expression> argument_;
};


class ExpressionStatement : public Statement {
public:
    ExpressionStatement(SourcePos start, SourcePos end, std::unique_ptr<Expression> expression)
        : Statement(start, end), expression_(std::move(expression)) {}

    StatementType type() const noexcept override { return StatementType::kExpression; }

    const std::unique_ptr<Expression>& expression() const { return expression_; }
    
    Statement* clone() const override {
        return new ExpressionStatement(start(), end(),
            expression_ ? std::unique_ptr<Expression>(static_cast<Expression*>(expression_->clone())) : nullptr);
    }

private:
    std::unique_ptr<Expression> expression_;
};


enum class PredefinedTypeKeyword {
    kNumber,
    kString,
    kBoolean,
    kAny,
    kVoid,
};

class Type : public Statement {
public:
    Type(SourcePos start, SourcePos end)
        : Statement(start, end) {}
    
    virtual Statement* clone() const override = 0;
};

class PredefinedType : public Type {
public:
    PredefinedType(SourcePos start, SourcePos end, PredefinedTypeKeyword keyword)
        : Type(start, end), keyword_(keyword) {}

    StatementType type() const noexcept override { return StatementType::kPredefinedType; }

    const PredefinedTypeKeyword& keyword() const { return keyword_; }
    
    Statement* clone() const override {
        return new PredefinedType(start(), end(), keyword_);
    }

private:
    PredefinedTypeKeyword keyword_;
};

class NamedType : public Type {
public:
    NamedType(SourcePos start, SourcePos end, std::string&& name)
        : Type(start, end), name_(std::move(name)) {}

    StatementType type() const noexcept override { return StatementType::kNamedType; }

    const std::string& name() const { return name_; }
    
    Statement* clone() const override {
        return new NamedType(start(), end(), std::string(name_));
    }

private:
    std::string name_;
};

class LiteralType : public Type {
public:
    LiteralType(SourcePos start, SourcePos end, std::unique_ptr<Expression>&& value)
        : Type(start, end), value_(std::move(value)) {}

    StatementType type() const noexcept override { return StatementType::kLiteralType; }

    const std::unique_ptr<Expression>& value() const { return value_; }
    
    Statement* clone() const override {
        return new LiteralType(start(), end(), 
            std::unique_ptr<Expression>(static_cast<Expression*>(value_->clone())));
    }

private:
    std::unique_ptr<Expression> value_;
};

class UnionType : public Type {
public:
    UnionType(SourcePos start, SourcePos end, std::vector<std::unique_ptr<Type>>&& types)
        : Type(start, end), types_(std::move(types)) {}

    StatementType type() const noexcept override { return StatementType::kUnionType; }

    const std::vector<std::unique_ptr<Type>>& types() const { return types_; }
    
    Statement* clone() const override {
        std::vector<std::unique_ptr<Type>> cloned_types;
        for (const auto& type : types_) {
            cloned_types.push_back(std::unique_ptr<Type>(static_cast<Type*>(type->clone())));
        }
        return new UnionType(start(), end(), std::move(cloned_types));
    }

private:
    std::vector<std::unique_ptr<Type>> types_;
};

class FunctionType : public Type {
private:
    std::unique_ptr<Type> return_types_;
    std::vector<std::unique_ptr<Type>> param_types_;
};

class TypeAnnotation : public Statement {
public:
    TypeAnnotation(SourcePos start, SourcePos end, std::unique_ptr<Type>&& type_p)
        : Statement(start, end), type_p_(std::move(type_p)) {}

    StatementType type() const noexcept override { return StatementType::kTypeAnnotation; }

    const std::unique_ptr<Type>& type_p() const { return type_p_; }
    
    Statement* clone() const override {
        return new TypeAnnotation(start(), end(), 
            std::unique_ptr<Type>(static_cast<Type*>(type_p_->clone())));
    }

private:
    std::unique_ptr<Type> type_p_;
};

} // namespace compiler
} // namespace mjs
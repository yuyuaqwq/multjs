#pragma once

#include <vector>
#include <memory>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/function_def.h>
#include <mjs/class_def/class_def.h>

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
    kLieralType,
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

private:
    SourcePos start_;
    SourcePos end_;
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

private:
    std::unique_ptr<Expression> argument_;
};


class ExpressionStatement : public Statement {
public:
    ExpressionStatement(SourcePos start, SourcePos end, std::unique_ptr<Expression> expression)
        : Statement(start, end), expression_(std::move(expression)) {}

    StatementType type() const noexcept override { return StatementType::kExpression; }

    const std::unique_ptr<Expression>& expression() const { return expression_; }

private:
    std::unique_ptr<Expression> expression_;
};

class BlockStatement : public Statement {
public:
    BlockStatement(SourcePos start, SourcePos end, std::vector<std::unique_ptr<Statement>>&& statements)
        : Statement(start, end), statements_(std::move(statements)) {}

    StatementType type() const noexcept override { return StatementType::kBlock; }

    const std::vector<std::unique_ptr<Statement>>& statements() const { return statements_; }

private:
    std::vector<std::unique_ptr<Statement>> statements_;
};



enum class PredefinedTypeKeyword {
    kNumber,
    kString,
    kBoolean,
    kAny,
    kVoid,
};

class Type : public Statement {
    using Statement::Statement;
};

class PredefinedType : public Type {
public:
    PredefinedType(SourcePos start, SourcePos end, PredefinedTypeKeyword keyword)
        : Type(start, end), keyword_(keyword) {}

    StatementType type() const noexcept override { return StatementType::kPredefinedType; }

    const PredefinedTypeKeyword& keyword() const { return keyword_; }

private:
    PredefinedTypeKeyword keyword_;
};

class NamedType : public Type {
public:
    NamedType(SourcePos start, SourcePos end, std::string&& name)
        : Type(start, end), name_(std::move(name)) {}

    StatementType type() const noexcept override { return StatementType::kNamedType; }

private:
    std::string name_;
    std::vector<std::unique_ptr<Type>> upper_types_;
};

class LieralType : public Type {
public:
    LieralType(SourcePos start, SourcePos end, std::unique_ptr<Expression>&& value)
        : Type(start, end), value_(std::move(value)) {}

    StatementType type() const noexcept override { return StatementType::kLieralType; }

private:
    std::unique_ptr<Expression> value_;
};

class UnionType : public Type {
public:
    UnionType(SourcePos start, SourcePos end, std::vector<std::unique_ptr<Type>>&& types)
        : Type(start, end), types_(std::move(types)) {}

    StatementType type() const noexcept override { return StatementType::kUnionType; }

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

private:
    std::unique_ptr<Type> type_p_;
};

} // namespace compiler
} // namespace mjs
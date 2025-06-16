#include "expression.h"

#include "statement.h"

namespace mjs {
namespace compiler {

Expression* UndefinedLiteral::clone() const {
    return new UndefinedLiteral(start(), end());
}

Expression* NullLiteral::clone() const {
    return new NullLiteral(start(), end());
}

Expression* BooleanLiteral::clone() const {
    return new BooleanLiteral(start(), end(), value_);
}

Expression* IntegerLiteral::clone() const {
    return new IntegerLiteral(start(), end(), value_);
}

Expression* FloatLiteral::clone() const {
    return new FloatLiteral(start(), end(), value_);
}

Expression* StringLiteral::clone() const {
    return new StringLiteral(start(), end(), std::string(value_));
}

Expression* ThisExpression::clone() const {
    return new ThisExpression(start(), end());
}

Expression* TemplateElement::clone() const {
    return new TemplateElement(start(), end(), std::string(value_));
}

Expression* TemplateLiteral::clone() const {
    std::vector<std::unique_ptr<Expression>> cloned_expressions;
    for (const auto& expr : expressions_) {
        cloned_expressions.push_back(std::unique_ptr<Expression>(expr->clone()));
    }
    return new TemplateLiteral(start(), end(), std::move(cloned_expressions));
}

Expression* ArrayExpression::clone() const {
    std::vector<std::unique_ptr<Expression>> cloned_elements;
    for (const auto& elem : elements_) {
        cloned_elements.push_back(std::unique_ptr<Expression>(elem->clone()));
    }
    return new ArrayExpression(start(), end(), std::move(cloned_elements));
}

Expression* ObjectExpression::clone() const {
    std::vector<Property> cloned_props;
    for (const auto& prop : properties_) {
        Property cloned_prop;
        cloned_prop.key = prop.key;
        cloned_prop.value = std::unique_ptr<Expression>(prop.value->clone());
        cloned_prop.shorthand = prop.shorthand;
        cloned_prop.computed = prop.computed;
        cloned_props.push_back(std::move(cloned_prop));
    }
    return new ObjectExpression(start(), end(), std::move(cloned_props));
}

Expression* FunctionExpression::clone() const {
    auto result = new FunctionExpression(start(), end(), id_, 
        std::vector<std::string>(params_),
        std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(body_->clone())),
        is_generator_, is_async_, is_module_);
    result->set_is_export(is_export_);
    return result;
}

Expression* ArrowFunctionExpression::clone() const {
    return new ArrowFunctionExpression(start(), end(),
        std::vector<std::string>(params_),
        std::unique_ptr<Statement>(body_->clone()),
        is_async_);
}

Expression* MemberExpression::clone() const {
    return new MemberExpression(start(), end(),
        std::unique_ptr<Expression>(object_->clone()),
        std::unique_ptr<Expression>(property_->clone()),
        is_method_call_, computed_, optional_);
}

Expression* CallExpression::clone() const {
    std::vector<std::unique_ptr<Expression>> cloned_args;
    for (const auto& arg : arguments_) {
        cloned_args.push_back(std::unique_ptr<Expression>(arg->clone()));
    }
    return new CallExpression(start(), end(),
        std::unique_ptr<Expression>(callee_->clone()),
        std::move(cloned_args));
}

Expression* NewExpression::clone() const {
    std::vector<std::unique_ptr<Expression>> cloned_args;
    for (const auto& arg : arguments_) {
        cloned_args.push_back(std::unique_ptr<Expression>(arg->clone()));
    }
    return new NewExpression(start(), end(),
        std::unique_ptr<Expression>(callee_->clone()),
        std::move(cloned_args));
}

Expression* UnaryExpression::clone() const {
    return new UnaryExpression(start(), end(), operator_,
        std::unique_ptr<Expression>(argument_->clone()),
        is_prefix_);
}

Expression* BinaryExpression::clone() const {
    return new BinaryExpression(start(), end(), operator_,
        std::unique_ptr<Expression>(left_->clone()),
        std::unique_ptr<Expression>(right_->clone()));
}

Expression* AssignmentExpression::clone() const {
    return new AssignmentExpression(start(), end(), operator_,
        std::unique_ptr<Expression>(left_->clone()),
        std::unique_ptr<Expression>(right_->clone()));
}

Expression* ConditionalExpression::clone() const {
    return new ConditionalExpression(start(), end(),
        std::unique_ptr<Expression>(test_->clone()),
        std::unique_ptr<Expression>(consequent_->clone()),
        std::unique_ptr<Expression>(alternate_->clone()));
}

Expression* YieldExpression::clone() const {
    return new YieldExpression(start(), end(),
        std::unique_ptr<Expression>(argument_->clone()));
}

Expression* AwaitExpression::clone() const {
    return new AwaitExpression(start(), end(),
        std::unique_ptr<Expression>(argument_->clone()));
}

Expression* ImportExpression::clone() const {
    return new ImportExpression(start(), end(),
        std::unique_ptr<Expression>(source_->clone()));
}

Expression* Identifier::clone() const {
    return new Identifier(start(), end(), std::string(name_));
}

} // namespace compiler
} // namespace mjs
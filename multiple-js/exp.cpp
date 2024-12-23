#include "exp.h"

namespace mjs {

ExpType NullExp::GetType() const noexcept {
	return ExpType::kNull;
}

ExpType BoolExp::GetType() const noexcept {
	return ExpType::kBool;
}
BoolExp::BoolExp(bool value) noexcept
	: value(value) {}

ExpType NumberExp::GetType() const noexcept {
	return ExpType::kNumber;
}
NumberExp::NumberExp(double value) noexcept
	: value(value) {}

ExpType StringExp::GetType() const noexcept {
	return ExpType::kString;
}
StringExp::StringExp(const std::string& value) 
	: value(value) {}


ExpType BinaOpExp::GetType() const noexcept {
	return ExpType::kBinaOp;
}

BinaOpExp::BinaOpExp(std::unique_ptr<Exp> lefexp, TokenType oper, std::unique_ptr<Exp> righexp)
	: left_exp(std::move(lefexp))
	, oper(oper)
	, right_exp(std::move(righexp)) {}

ExpType NameExp::GetType() const noexcept {
	return ExpType::kName;
}

NameExp::NameExp(const std::string& name) :
	name(name) {}

ExpType FunctionCallExp::GetType() const noexcept {
	return ExpType::kFunctionCall;
}

FunctionCallExp::FunctionCallExp(const std::string& name, std::vector<std::unique_ptr<Exp>>&& par_list)
	: name(name)
	, par_list(std::move(par_list)) {}

} // namespace mjs
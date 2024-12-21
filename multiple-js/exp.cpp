#include "exp.h"

namespace mjs {

ExpType NullExp::GetType() const noexcept {
	return ExpType::kNull;
}

ExpType BoolExp::GetType() const noexcept {
	return ExpType::kBool;
}
BoolExp::BoolExp(bool t_value) noexcept
	: value(t_value) {}

ExpType NumberExp::GetType() const noexcept {
	return ExpType::kNumber;
}
NumberExp::NumberExp(int t_value) noexcept
	: value(t_value) {}

ExpType StringExp::GetType() const noexcept {
	return ExpType::kString;
}
StringExp::StringExp(const std::string& t_value) 
	: value(t_value) {}


ExpType BinaOpExp::GetType() const noexcept {
	return ExpType::kBinaOp;
}

BinaOpExp::BinaOpExp(std::unique_ptr<Exp> left_exp, TokenType oper, std::unique_ptr<Exp> right_exp)
	: left_exp(std::move(left_exp))
	, oper(oper)
	, right_exp(std::move(right_exp)) {}

ExpType NameExp::GetType() const noexcept {
	return ExpType::kName;
}

NameExp::NameExp(const std::string& t_name) :
	name(t_name) {}

ExpType FunctionCallExp::GetType() const noexcept {
	return ExpType::kFunctionCall;
}

FunctionCallExp::FunctionCallExp(const std::string& name, std::vector<std::unique_ptr<Exp>>&& par_list)
	: name(name)
	, par_list(std::move(par_list)) {}

} // namespace mjs
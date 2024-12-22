#pragma once

#include <vector>
#include <memory>

#include "token.h"

namespace mjs {

enum class ExpType {
	kNull,
	kBool,
	kNumber,
	kString,
	kBinaOp,
	kName,
	kFunctionCall,
};

struct Exp {
	virtual ExpType GetType() const noexcept = 0;
};

struct NullExp : public Exp {
	virtual ExpType GetType() const noexcept;
};

struct BoolExp : public Exp {
	virtual ExpType GetType() const noexcept;
	BoolExp(bool t_value) noexcept;

	bool value;
};

struct NumberExp : public Exp {
	virtual ExpType GetType() const noexcept;
	NumberExp(int t_value) noexcept;

	int64_t value;
};

struct StringExp : public Exp {
	virtual ExpType GetType() const noexcept;
	StringExp(const std::string& t_value);

	std::string value;
};

struct BinaOpExp : public Exp {
	virtual ExpType GetType() const noexcept;
	BinaOpExp(std::unique_ptr<Exp> left_exp, TokenType oper, std::unique_ptr<Exp> right_exp);

	std::unique_ptr<Exp> left_exp;
	TokenType oper;
	std::unique_ptr<Exp> right_exp;
};

struct NameExp : public Exp {
	virtual ExpType GetType() const noexcept;
	NameExp(const std::string& t_name);

	std::string name;
};


struct FunctionCallExp : public Exp {
	virtual ExpType GetType() const noexcept;
	FunctionCallExp(const std::string& name, std::vector<std::unique_ptr<Exp>>&& par_list);

	std::string name;
	std::vector<std::unique_ptr<Exp>> par_list;
};

} // namespace mjs
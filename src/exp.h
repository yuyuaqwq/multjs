#pragma once

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

namespace mjs {

enum class ExpType {
	kNull,
	kBool,
	kNumber,
	kString,
	kUnaryOp,
	kBinaryOp,
	kTernaryOp,
	kIdentifier,
	kArrayLiteralExp,
	kObjectLiteralExp,
	kIndexedExp,
	kFunctionCall,
};

enum class ExpValueCategory {
	kLeftValue,
	kRightValue,
};

struct Exp {
	virtual ExpType GetType() const noexcept = 0;
	ExpValueCategory value_category = ExpValueCategory::kRightValue;
};

struct NullExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kNull;
	}
};

struct BoolExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kBool;
	}
	BoolExp(bool value) noexcept
		: value(value) {}

	bool value;
};

struct NumberExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kNumber;
	}
	NumberExp(double value) noexcept
		: value(value) {}

	double value;
};

struct StringExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kString;
	}
	StringExp(const std::string& value)
		: value(value) {}


	std::string value;
};

struct UnaryOpExp : public Exp {
	UnaryOpExp(TokenType oper, std::unique_ptr<Exp> operand)
		: oper(oper)
		, operand(std::move(operand)) {}

	virtual ExpType GetType() const noexcept override {
		return ExpType::kUnaryOp;
	}

	TokenType oper;
	std::unique_ptr<Exp> operand;
};


struct BinaryOpExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kBinaryOp;
	}
	BinaryOpExp(std::unique_ptr<Exp> left_exp, TokenType oper, std::unique_ptr<Exp> right_exp)
		: left_exp(std::move(left_exp))
		, oper(oper)
		, right_exp(std::move(right_exp)) {}

	std::unique_ptr<Exp> left_exp;
	TokenType oper;
	std::unique_ptr<Exp> right_exp;
};

struct TernaryOpExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kTernaryOp;
	}
	TernaryOpExp(TokenType oper, std::unique_ptr<Exp> exp1, std::unique_ptr<Exp> exp2, std::unique_ptr<Exp> exp3)
		: oper(oper)
		, exp1(std::move(exp1))
		, exp2(std::move(exp2))
		, exp3(std::move(exp3)) {}

	TokenType oper;
	std::unique_ptr<Exp> exp1;
	std::unique_ptr<Exp> exp2;
	std::unique_ptr<Exp> exp3;
};

struct IdentifierExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kIdentifier;
	}
	IdentifierExp(const std::string& name) :
		name(name)
	{
		value_category = ExpValueCategory::kLeftValue;
	}

	std::string name;
};

struct IndexedExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kIndexedExp;
	}
	IndexedExp(std::unique_ptr<Exp> exp, std::unique_ptr<Exp> index_exp)
		: exp(std::move(exp))
		, index_exp(std::move(index_exp)) {}

	std::unique_ptr<Exp> exp;
	std::unique_ptr<Exp> index_exp;
};

struct ArrayLiteralExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kArrayLiteralExp;
	}
	ArrayLiteralExp(std::vector<std::unique_ptr<Exp>>&& arr_litera)
		: arr_litera(std::move(arr_litera)) {}

	std::vector<std::unique_ptr<Exp>> arr_litera;
};

struct ObjectLiteralExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kObjectLiteralExp;
	}
	ObjectLiteralExp(std::unordered_map<std::string, std::unique_ptr<Exp>>&& obj_litera)
		: obj_litera(std::move(obj_litera)) {}

	std::unordered_map<std::string, std::unique_ptr<Exp>> obj_litera;
};


struct FunctionCallExp : public Exp {
	virtual ExpType GetType() const noexcept override {
		return ExpType::kFunctionCall;
	}
	FunctionCallExp(std::unique_ptr<Exp> func_name, std::vector<std::unique_ptr<Exp>>&& par_list)
		: func_name(std::move(func_name))
		, par_list(std::move(par_list)) {}

	std::unique_ptr<Exp> func_name;
	std::vector<std::unique_ptr<Exp>> par_list;
};

} // namespace mjs
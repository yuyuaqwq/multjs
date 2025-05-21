#pragma once

#include <string>
#include <format>
#include <utility>
#include <exception>

namespace mjs {

class Error : public std::exception {
public:
	Error(std::string info) {
		info_ = info;
	}

	template<typename... Args>
	Error(std::format_string<Args...> fmt, Args&&... args) {
		info_ = std::format(fmt, std::forward<Args>(args)...);
	}

	virtual const char* error_name() const {
		return "Error";
	}

	virtual const char* what() const override {
		if (!error_name_) {
			auto mut_this = const_cast<Error*>(this);
			mut_this->info_ = std::string(error_name()) + ": " + info_;
			mut_this->error_name_ = true;
		}
		return info_.c_str();
	}

private:
	bool error_name_ = false;
	std::string info_;
};

class InternalError : public Error {
public:
	using Error::Error;

	const char* error_name() const override {
		return "InternalError";
	}
};

class RangeError : public Error {
public:
	using Error::Error;

	const char* error_name() const override {
		return "RangeError";
	}
};

class ReferenceError : public Error {
public:
	using Error::Error;

	const char* error_name() const override {
		return "ReferenceError";
	}
};

class SyntaxError : public Error {
public:
	using Error::Error;

	const char* error_name() const override {
		return "SyntaxError";
	}
};

class TypeError : public Error {
public:
	using Error::Error;

	const char* error_name() const override {
		return "TypeError";
	}
};

} // namespace mjs
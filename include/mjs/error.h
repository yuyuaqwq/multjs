#pragma once

#include <string>
#include <format>
#include <utility>
#include <exception>

#include <mjs/value.h>

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
		return info_.c_str();
	}

	template<typename... Args>
	static Value Throw(std::format_string<Args...> fmt, Args&&... args) {
		return Value(String::New(std::format(fmt, std::forward<Args>(args)...))).SetException();
	}

private:
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
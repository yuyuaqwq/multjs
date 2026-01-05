#pragma once

#include <string>
#include <format>
#include <utility>
#include <exception>

#include <mjs/value.h>

namespace mjs {

class Context;
class StackFrame;
class Error : public std::exception {
public:
	virtual ~Error() = default;

	virtual const char* error_name() const {
		return "Error";
	}

	Pc error_pc() const {
		return error_pc_;
	}

	virtual const char* what() const override {
		return info_.c_str();
	}

	template<typename... Args>
	static Value Throw(Context* context, std::format_string<Args...> fmt, Args&&... args) {
		return Value(String::Format(fmt, std::forward<Args>(args)...)).SetException();
	}

protected:
	template<typename... Args>
	Error(Pc error_pc, std::format_string<Args...> fmt, Args&&... args) {
		error_pc_ = error_pc;
		info_ = std::format(fmt, std::forward<Args>(args)...);
	}

private:
	Pc error_pc_;
	std::string info_;
};

class SyntaxError : public Error {
public:
	template<typename... Args>
	SyntaxError(std::format_string<Args...> fmt, Args&&... args)
		: Error(kInvalidPc, std::move(fmt), std::forward<Args>(args)...) {}

	const char* error_name() const override {
		return "SyntaxError";
	}
};

class InternalError : public Error {
public:
	template<typename... Args>
	InternalError(std::format_string<Args...> fmt, Args&&... args)
		: Error(kInvalidPc, std::move(fmt), std::forward<Args>(args)...) {}

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

class TypeError : public Error {
public:
	template<typename... Args>
	TypeError(std::format_string<Args...> fmt, Args&&... args)
		: Error(kInvalidPc, std::move(fmt), std::forward<Args>(args)...) {}

	const char* error_name() const override {
		return "TypeError";
	}
};

} // namespace mjs
#include <mjs/class_def/string_object_class_def.h>

#include <string>
#include <vector>
#include <algorithm>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/value/object/array_object.h>

namespace mjs {

StringObjectClassDef::StringObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kStringObject, "String")
{
	// String.prototype.__proto__ = Object.prototype
	prototype_.object().SetPrototype(&runtime->default_context(), runtime->class_def_table()[ClassId::kObject].prototype());
	// String.__proto = Function.prototype
	constructor_.object().SetPrototype(&runtime->default_context(), runtime->class_def_table()[ClassId::kFunctionObject].prototype());

	// Split method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kSplit, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			GCHandleScope<1> scope(context);
			auto array = scope.New<ArrayObject>();
			return scope.Close(array);
		}
		std::string str = stack.this_val().ToString(context).string_view();
		std::string delimiter = stack.get(0).ToString(context).string_view();

		GCHandleScope<1> scope(context);
		auto array = scope.New<ArrayObject>(0);
		if (delimiter.empty()) {
			for (char c : str) {
				array->Push(context, Value(String::New(std::string(1, c))));
			}
		}
		else {
			size_t pos = 0;
			while ((pos = str.find(delimiter)) != std::string::npos) {
				array->Push(context, Value(String::New(str.begin(), str.begin() + pos)));
				str.erase(0, pos + delimiter.length());
			}
			array->Push(context, Value(String::New(str)));
		}
		return scope.Close(array);
	}));

	// Substring method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kSubString, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		int start = 0;
		int end = str.length();
		
		if (par_count > 0) {
			start = static_cast<int>(stack.get(0).ToNumber().f64());
			if (start < 0) start = 0;
			if (start > str.length()) start = str.length();
		}
		
		if (par_count > 1) {
			end = static_cast<int>(stack.get(1).ToNumber().f64());
			if (end < 0) end = 0;
			if (end > str.length()) end = str.length();
		}
		
		if (start > end) std::swap(start, end);
		return Value(String::New(str.begin() + start, str.begin() + (end - start)));
	}));

	// IndexOf method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kIndexOf, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) return Value(-1);
		
		std::string str = stack.this_val().ToString(context).string_view();
		std::string searchStr = stack.get(0).ToString(context).string_view();
		int startPos = 0;
		
		if (par_count > 1) {
			startPos = static_cast<int>(stack.get(1).ToNumber().f64());
			if (startPos < 0) startPos = 0;
		}
		
		size_t pos = str.find(searchStr, startPos);
		return Value(pos == std::string::npos ? -1 : static_cast<int>(pos));
	}));

	// ToLowerCase method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kToLowerCase, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		std::string result;
		result.reserve(str.length());
		for (char c : str) {
			result += std::tolower(c);
		}
		return Value(String::New(result));
	}));

	// ToUpperCase method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kToUpperCase, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		std::string result;
		result.reserve(str.length());
		for (char c : str) {
			result += std::toupper(c);
		}
		return Value(String::New(result));
	}));

	// Trim method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kTrim, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		auto start = str.find_first_not_of(" \t\n\r\f\v");
		if (start == std::string::npos) return Value("");
		auto end = str.find_last_not_of(" \t\n\r\f\v");
		return Value(String::New(str.begin() + start, str.begin() + (end - start + 1)));
	}));

	// Replace method
	prototype_.object().SetProperty(&runtime->default_context(), ConstIndexEmbedded::kReplace, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 2) return stack.this_val();
		
		std::string str = stack.this_val().ToString(context).string_view();
		std::string searchStr = stack.get(0).ToString(context).string_view();
		std::string replaceStr = stack.get(1).ToString(context).string_view();
		
		size_t pos = str.find(searchStr);
		if (pos == std::string::npos) return Value(String::New(str));
		
		return Value(String::New(str.substr(0, pos) + replaceStr + str.substr(pos + searchStr.length())));
	}));
}

} // namespace mjs
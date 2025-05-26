#include <mjs/class_def_impl/string_object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/object_impl/array_object.h>
#include <string>
#include <vector>
#include <algorithm>

namespace mjs {

StringObjectClassDef::StringObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kStringObject, "String")
{
	// Split method
	auto split_index = runtime->const_pool().insert(Value("split"));
	prototype_.object().SetProperty(nullptr, split_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return Value(ArrayObject::New(context, {}));
		}
		std::string str = stack.this_val().ToString(context).string_view();
		std::string delimiter = stack.get(0).ToString(context).string_view();
		
		auto array = ArrayObject::New(context, 0);
		if (delimiter.empty()) {
			for (char c : str) {
				array->Push(context, Value(String::New(std::string(1, c))));
			}
		} else {
			size_t pos = 0;
			std::string token;
			while ((pos = str.find(delimiter)) != std::string::npos) {
				token = str.substr(0, pos);
				array->Push(context, Value(String::New(token)));
				str.erase(0, pos + delimiter.length());
			}
			array->Push(context, Value(String::New(str)));
		}
		
		return Value(array);
	}));

	// Substring method
	auto substring_index = runtime->const_pool().insert(Value("substring"));
	prototype_.object().SetProperty(nullptr, substring_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
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
		return Value(str.substr(start, end - start));
	}));

	// IndexOf method
	auto indexOf_index = runtime->const_pool().insert(Value("indexOf"));
	prototype_.object().SetProperty(nullptr, indexOf_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
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
	auto toLowerCase_index = runtime->const_pool().insert(Value("toLowerCase"));
	prototype_.object().SetProperty(nullptr, toLowerCase_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		std::string result;
		result.reserve(str.length());
		for (char c : str) {
			result += std::tolower(c);
		}
		return Value(result);
	}));

	// ToUpperCase method
	auto toUpperCase_index = runtime->const_pool().insert(Value("toUpperCase"));
	prototype_.object().SetProperty(nullptr, toUpperCase_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		std::string result;
		result.reserve(str.length());
		for (char c : str) {
			result += std::toupper(c);
		}
		return Value(result);
	}));

	// Trim method
	auto trim_index = runtime->const_pool().insert(Value("trim"));
	prototype_.object().SetProperty(nullptr, trim_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		std::string str = stack.this_val().ToString(context).string_view();
		auto start = str.find_first_not_of(" \t\n\r\f\v");
		if (start == std::string::npos) return Value(std::string());
		auto end = str.find_last_not_of(" \t\n\r\f\v");
		return Value(str.substr(start, end - start + 1));
	}));

	// Replace method
	auto replace_index = runtime->const_pool().insert(Value("replace"));
	prototype_.object().SetProperty(nullptr, replace_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 2) return stack.this_val();
		
		std::string str = stack.this_val().ToString(context).string_view();
		std::string searchStr = stack.get(0).ToString(context).string_view();
		std::string replaceStr = stack.get(1).ToString(context).string_view();
		
		size_t pos = str.find(searchStr);
		if (pos == std::string::npos) return Value(str);
		
		return Value(str.substr(0, pos) + replaceStr + str.substr(pos + searchStr.length()));
	}));

	// Length property
	//auto length_index = runtime->const_pool().insert(Value("length"));
	//prototype_.object().SetProperty(nullptr, length_index, Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
	//	return Value(static_cast<int64_t>(stack.this_val().ToString(context).string_view().length()));
	//}));
}

} // namespace mjs
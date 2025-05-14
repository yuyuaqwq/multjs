#pragma once

#include <string_view>
#include <format>

#include <mjs/reference_counter.h>

namespace mjs {

// 不会有循环引用问题，仅使用引用计数管理
class String : public ReferenceCounter<String> {
public:
	String(size_t size) 
		: size_(size) {}
	
	size_t hash() const {
		return hash_;
	}

	const char* data() const {
		return data_;
	}

	bool empty() const {
		return size_ == 0;
	}


	template<typename... Args>
	static String* Format(std::format_string<Args...> fmt, Args&&... args) {
		// First, format into a temporary buffer to determine the size
		const auto size = std::formatted_size(fmt, std::forward<Args>(args)...);

		// Allocate memory for String object plus space for the formatted string
		String* s = static_cast<String*>(::operator new(sizeof(String) + size + 1));

		// Use placement new to construct the String object
		new (s) String(size);

		// Format directly into our data_ buffer
		std::format_to_n(s->data_, size + 1, fmt, std::forward<Args>(args)...);
		s->data_[size] = '\0';  // Ensure null-termination

		// Calculate hash based on the formatted content
		std::string_view view(s->data_, size);
		s->hash_ = std::hash<std::string_view>()(view);

		return s;
	}


	static String* New(std::string_view str) {
		auto size = str.size();
		// Allocate memory for String object plus space for the string data
		String* s = static_cast<String*>(::operator new(sizeof(String) + size + 1));
		// Use placement new to construct the String object
		new (s) String(size);
		// Copy the string data
		memcpy(s->data_, str.data(), size + 1);
		// Calculate the hash based on the actual string content
		s->hash_ = std::hash<std::string_view>()(str);
		return s;
	}

private:
	size_t hash_ = 0;
	size_t size_;
	char data_[];
};

} // namespace mjs
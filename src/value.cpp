#include <mjs/value.h>

namespace mjs {

const char* Value::string_u8() const {
	assert(type() == ValueType::kString);
	if (tag_.string_length_ < sizeof(value_.string_u8_inline_)) {
		return value_.string_u8_inline_;
	}
	else {
		return static_cast<StringObject*>(value_.object_)->str();
	}
}

void Value::set_string_u8(const char* string_u8, size_t size) {
	if (size < sizeof(value_.string_u8_inline_)) {
		std::memcpy(value_.string_u8_inline_, string_u8, size);
		value_.string_u8_inline_[size] = '\0';
	}
	else {
		StringObject* str_obj = static_cast<StringObject*>(std::malloc(sizeof(StringObject) + size - 8 + 1));
		std::construct_at(str_obj);
		std::memcpy(str_obj->mutable_str(), string_u8, size);
		str_obj->mutable_str()[size] = '\0';
		value_.object_ = str_obj;
		str_obj->ref();
	}
	tag_.string_length_ = size;
}

} // namespace mjs